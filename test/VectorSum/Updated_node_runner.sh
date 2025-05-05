#!/bin/bash
#SBATCH --job-name=VectorSumMPI
#SBATCH --output=logs/output_%j.log
#SBATCH --ntasks-per-node=1    #Voglio che ci sia solo un  task per node e lo forzo 
#SBATCH --nodes=2                # <-- UN SOLO nodo ora, se specifichi solo questo alloca due processori che lui pensa che usi poi se li gestisce lui
#SBATCH --time=00:10:00
#SBATCH --mail-type=BEGIN,END,FAIL
#SBATCH --mail-user=nicolo.venieri2@studio.unibo.it
#SBATCH --exclusive
# Creo cartella logs se non esiste
mkdir -p logs

# Carico i moduli
module load gcc/12.2.0
module load openmpi/4.1.6--gcc--12.2.0

# Mi sposto nella cartella degli script
cd $HOME/cineca_perftest/test/VectorSum/MPIscripts

# File di output per perf
PERF_OUTPUT="$HOME/cineca_perftest/test/VectorSum/logs/perf_outputs.txt"
touch $PERF_OUTPUT

# Ottengo i nodi utilizzati
NODES=$(scontrol show job $SLURM_JOB_ID | grep "Nodes" | awk -F= '{print $2}')

# Intestazione log
echo "Job Name: $SLURM_JOB_NAME" >> $PERF_OUTPUT
echo "Nodes Used: $NODES" >> $PERF_OUTPUT
echo "Job ID: $SLURM_JOB_ID" >> $PERF_OUTPUT
echo "Start Time: $(date)" >> $PERF_OUTPUT
echo "----------------------------------------" >> $PERF_OUTPUT

# Eseguo il programma MPI e monitoro energia
echo "Running MPI program with perf monitoring (energy)..." >> $PERF_OUTPUT
perf stat -e power/energy-pkg/ -A srun ./mpi_energy_logger | tee -a $PERF_OUTPUT

# Fine test
echo "End Time: $(date)" >> $PERF_OUTPUT
echo "----------------------------------------" >> $PERF_OUTPUT

# SE ESEGUI SU UN NODO SOLO TASK PER NODE 2 E NODES 1
#SE ESEGUI SU DUE NODI TASK PER NODES 1 E NODES 2
