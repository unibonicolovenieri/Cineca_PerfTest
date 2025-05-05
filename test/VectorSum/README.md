Certo, ecco il file Markdown che descrive ogni dettaglio dei tre file che abbiamo creato, con una spiegazione approfondita di ciascun componente e dei vari scenari in cui vengono utilizzati.

---

# **Documentazione del Progetto di Misurazione Energetica con MPI e RAPL**

Questo progetto ha come obiettivo la misurazione del consumo energetico durante l'esecuzione di un'applicazione MPI su un sistema multi-socket, utilizzando il framework Intel RAPL (Running Average Power Limit). Vengono descritti i tre file principali che compongono il progetto:

1. **`energy_meter.h`** - Header per la misurazione dell'energia.
2. **`energy_meter.c`** - Implementazione delle funzioni per la misurazione dell'energia.
3. **`mpi_energy_logger.c`** - Programma MPI che integra la misurazione dell'energia durante l'esecuzione parallela.
4. **`run_energy_mpi.slurm`** - Script SLURM per l'esecuzione del programma con monitoraggio dell'energia.

---

## **1. `energy_meter.h`**

Il file **`energy_meter.h`** definisce l'interfaccia per l'inizializzazione e il monitoraggio delle misurazioni energetiche, specificamente usando i registri RAPL di Intel.

### **Definizioni principali**

#### `energy_zone_t` (Enumerazione)

```c
typedef enum {
    ENERGY_PKG,   // Misura del consumo energetico del pacchetto CPU (tutti i core del socket)
    ENERGY_CORE,  // Misura del consumo energetico di un singolo core
    ENERGY_DRAM,  // Misura del consumo energetico della memoria DRAM
    ENERGY_PSYS   // Misura del consumo energetico della parte di sistema (compreso CPU, core, etc.)
} energy_zone_t;
```

Questa enumerazione definisce le possibili zone da cui raccogliere i dati di consumo energetico. Ogni tipo di zona è associato a una diversa parte del sistema hardware:

* `ENERGY_PKG`: Energia consumata dal pacchetto CPU (include tutti i core).
* `ENERGY_CORE`: Energia consumata da un singolo core.
* `ENERGY_DRAM`: Energia consumata dalla memoria DRAM.
* `ENERGY_PSYS`: Energia totale del sistema, che include anche altri componenti.

#### `energy_meter_t` (Struttura)

```c
typedef struct {
    int socket;           // Identificatore del socket
    energy_zone_t zone;   // Zona energetica da monitorare (vedi enumerazione)
    long long start;      // Valore iniziale dell'energia (al momento di start)
    long long stop;       // Valore finale dell'energia (al momento di stop)
    char path[256];       // Path al file di sistema RAPL
} energy_meter_t;
```

La struttura `energy_meter_t` rappresenta un misuratore di energia per un dato socket e zona. Contiene i seguenti campi:

* `socket`: Specifica il socket della CPU.
* `zone`: La zona da cui raccogliere i dati energetici.
* `start`: Il valore iniziale dell'energia (registrato al momento dell'inizio della misurazione).
* `stop`: Il valore finale dell'energia (registrato al momento del termine della misurazione).
* `path`: Il percorso del file di sistema RAPL da cui vengono letti i dati energetici.

### **Funzioni**

* `energy_meter_init_manual()`: Inizializza un misuratore di energia per un socket specificato e una zona specifica.
* `energy_meter_init()`: Inizializza un misuratore di energia rilevando automaticamente il socket corrente.
* `energy_meter_start()`: Avvia la misurazione dell'energia, salvando il valore iniziale.
* `energy_meter_stop()`: Ferma la misurazione dell'energia, salvando il valore finale.
* `energy_meter_joules()`: Calcola il consumo energetico in joule come differenza tra `start` e `stop`.

---

## **2. `energy_meter.c`**

Il file **`energy_meter.c`** implementa le funzioni dichiarate in `energy_meter.h`. Queste funzioni interagiscono direttamente con il sistema per leggere i dati energetici dai file di sistema RAPL.

### **Funzione `read_energy_uj()`**

```c
static long long read_energy_uj(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return -1;
    long long val = -1;
    fscanf(f, "%lld", &val);
    fclose(f);
    return val;
}
```

Questa funzione legge il valore energetico da un file di sistema. Il file rappresenta una zona energetica specifica (ad esempio, il pacchetto CPU). Restituisce il valore in microjoule.

### **Funzione `energy_meter_init()`**

```c
int energy_meter_init(energy_meter_t* m, energy_zone_t zone) {
    // Inizializza il misuratore di energia per una zona specifica
    // Costruisce dinamicamente il percorso del file RAPL.
}
```

Questa funzione inizializza un misuratore di energia per una zona specifica (`zone`), creando dinamicamente il percorso del file RAPL corrispondente alla zona. Ogni zona ha un percorso specifico per il file `energy_uj` che contiene i dati di consumo energetico.

### **Funzioni `energy_meter_start()` e `energy_meter_stop()`**

```c
int energy_meter_start(energy_meter_t* m);
int energy_meter_stop(energy_meter_t* m);
```

Queste funzioni leggono il valore dell'energia all'inizio (`start`) e alla fine (`stop`) del periodo di misurazione. Utilizzano il percorso configurato nella struttura `energy_meter_t`.

### **Funzione `energy_meter_joules()`**

```c
double energy_meter_joules(energy_meter_t* m);
```

Calcola e restituisce il consumo energetico in joule come differenza tra il valore di `stop` e `start`, convertito da microjoule a joule (1 microjoule = 1e-6 joule).

---

## **3. `mpi_energy_logger.c`**

Il file **`mpi_energy_logger.c`** è il programma MPI che integra il monitoraggio energetico. Esso avvia un'applicazione MPI, misura il consumo energetico durante l'esecuzione e scrive i dati in file di log separati per ogni processo.

### **Struttura del programma**

#### **Inizializzazione e avvio del programma**

```c
int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Creazione dei misuratori di energia
    energy_meter_t energy_meter;
    energy_meter_init(&energy_meter, ENERGY_PKG);

    if (rank == 0) {
        // Avvio della misurazione per il rank 0
        energy_meter_start(&energy_meter);
    }
    if (rank == 1) {
        // Avvio della misurazione per il rank 1
        energy_meter_start(&energy_meter);
    }

    // Esegui il calcolo (esempio: VectorSum)
    // MPI_Barrier(MPI_COMM_WORLD);
    // Esegui operazioni MPI...

    // Fine della misurazione
    if (rank == 0) {
        energy_meter_stop(&energy_meter);
        double joules = energy_meter_joules(&energy_meter);
        // Scrivi i dati nel file di log
    }

    if (rank == 1) {
        energy_meter_stop(&energy_meter);
        double joules = energy_meter_joules(&energy_meter);
        // Scrivi i dati nel file di log
    }

    MPI_Finalize();
}
```

### **Comportamento del programma:**

* Ogni rank (processo) esegue un ciclo di misurazione energetica prima e dopo il calcolo MPI.
* I dati di consumo energetico vengono salvati in file separati per ogni rank (rank 0 scrive nel file `rank_0_log.txt`, rank 1 nel file `rank_1_log.txt`).
* Utilizza `MPI_Init`, `MPI_Finalize`, `MPI_Comm_rank`, e `MPI_Comm_size` per gestire la distribuzione del lavoro e la comunicazione tra i processi.

---

## **4. `run_energy_mpi.slurm`**

Lo script **`run_energy_mpi.slurm`** è un file SLURM che automatizza l'esecuzione del programma MPI su un cluster. Esso include il caricamento dei moduli necessari, la creazione dei file di log e l'esecuzione del programma con il monitoraggio energetico tramite `perf`.

### **Dettagli dello script SLURM**

```bash
#!/bin/bash
#SBATCH --job-name=VectorSumMPI
#SBATCH --output=logs/output_%j.log
#SBATCH --ntasks-per-node=1
#SBATCH --nodes=2
#SBATCH --time=00:10:00
#SBATCH --mail-type=BEGIN,END,FAIL
#SBATCH --mail-user=nicolo.venieri2@studio.unibo.it
#SBATCH --exclusive
```

* **`--ntasks-per-node=1`**: Limita l'esecuzione a un solo task per nodo.
* **`--nodes=2`**: Esegui su 2 nodi.
* **`--time=00:10:00`**: Tempo di esecuzione massimo.
* **`--exclusive`**: Assicura che non ci siano altre attività sui nodi.

```bash
module load gcc/12.2.0
module load openmpi/4.1.6--gcc--12.2.0
```

Carica i moduli per il compilatore `gcc` e `openmpi`.

```bash
# Esegui il programma MPI con perf
perf stat -e power/energy-pkg/ -A srun ./mpi_energy_logger | tee -a $PERF_OUTPUT
```

* **`perf stat -e power/energy-pkg/`**: Utilizza `perf` per monitorare il consumo energetico del pacchetto CPU.
* **`srun ./mpi_energy_logger`**: Esegue il programma MPI.
* **`tee -a $PERF_OUTPUT`**: Appende l'output di `perf` a un file di log.

### **Creazione di Log Separati**

Lo script crea una cartella `logs/mpi_logs` per separare i log dei diversi rank, mantenendo traccia dei consumi energetici separatamente per ogni processo.

---

## **Conclusione**

Questa documentazione descrive in dettaglio il funzionamento dei tre file principali del progetto: `energy_meter.h`, `energy_meter.c`, e `mpi_energy_logger.c`, oltre allo script SLURM `run_energy_mpi.slurm`. Il progetto mira a monitorare il consumo energetico durante l'esecuzione di un'applicazione MPI su una macchina multi-socket, utilizzando i registri RAPL di Intel e strumenti come `perf` per raccogliere e scrivere i dati di energia in tempo reale.

Se hai bisogno di ulteriori chiarimenti o modifiche, sono a tua disposizione!

---
