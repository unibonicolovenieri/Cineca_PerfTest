#!/bin/bash
#SBATCH --job-name=VectorSumMPI         # Nome del job
#SBATCH --output=logs/output_%j.log     # Nome del file di output (log) nella cartella logs
#SBATCH --ntasks=2                      # Numero di processi (MPI)
#SBATCH --nodes=1                       # Numero di nodi (se necessario)
#SBATCH --time=00:10:00                 # Tempo massimo di esecuzione
#SBATCH --mail-type=BEGIN,END,FAIL      # Notifiche quando il job inizia, finisce, o fallisce
#SBATCH --mail-user=nicolo.venieri2@studio.unibo.it  # La tua email
#SBATCH --partition=boost_usr_prod     # Partizione da utilizzare (sostituisci con quella che desideri)

# Crea la cartella logs se non esiste già
mkdir -p logs

# Carica il modulo GCC e OpenMPI
module load gcc/12.2.0
module load openmpi/4.1.6--gcc--12.2.0

# Naviga alla cartella dove c'è il codice MPI (usa la tua directory FAST)
cd $FAST/VectorSum/MPIscripts  # La cartella dove hai copiato i tuoi script

# Compila il programma MPI
mpirun --bind-to core --overload-allowed -np 2 ./mpi_vector_sum

# Crea un file per l'output di perf
PERF_OUTPUT="$FAST/VectorSum/logs/perf_outputs.txt"

# Ottieni i nodi utilizzati
NODES=$(scontrol show job $SLURM_JOB_ID | grep "Nodes" | awk -F= '{print $2}')

# Scrivi l'intestazione con il nome del job e i nodi utilizzati
echo "Job Name: $SLURM_JOB_NAME" > $PERF_OUTPUT
echo "Nodes Used: $NODES" >> $PERF_OUTPUT
echo "Job ID: $SLURM_JOB_ID" >> $PERF_OUTPUT
echo "Start Time: $(date)" >> $PERF_OUTPUT
echo "----------------------------------------" >> $PERF_OUTPUT

# Esegui il programma MPI con 2 processi e calcola la somma dei vettori
mpirun -np 2 ./mpi_vector_sum

# Esegui il comando perf dopo il programma MPI
echo "Running perf after MPI..." >> $PERF_OUTPUT
perf stat -e task-clock,context-switches,cpu-migrations,page-faults,cycles,instructions,branches,branch-misses -p $$ >> $PERF_OUTPUT 2>&1 &

# Aggiungi la fine del test
echo "End Time: $(date)" >> $PERF_OUTPUT
echo "----------------------------------------" >> $PERF_OUTPUT

