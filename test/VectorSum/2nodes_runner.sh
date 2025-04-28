#!/bin/bash
#SBATCH --job-name=VectorSumMPI
#SBATCH --output=logs/output_%j.log
#SBATCH --ntasks=2
#SBATCH --nodes=2                # <-- DUE nodi adesso
#SBATCH --time=00:10:00
#SBATCH --mail-type=BEGIN,END,FAIL
#SBATCH --mail-user=nicolo.venieri2@studio.unibo.it

# Creo cartella logs se non esiste
mkdir -p logs

# Carico i moduli
module load gcc/12.2.0
module load openmpi/4.1.6--gcc--12.2.0

# Mi sposto nella cartella degli script
cd $FAST/VectorSum/MPIscripts

# File di output per perf
PERF_OUTPUT="$FAST/VectorSum/logs/perf_outputs.txt"
touch $PERF_OUTPUT

# Ottengo i nodi utilizzati
NODES=$(scontrol show job $SLURM_JOB_ID | grep "Nodes" | awk -F= '{print $2}')

# Intestazione log
echo "Job Name: $SLURM_JOB_NAME" >> $PERF_OUTPUT
echo "Nodes Used: $NODES" >> $PERF_OUTPUT
echo "Job ID: $SLURM_JOB_ID" >> $PERF_OUTPUT
echo "Start Time: $(date)" >> $PERF_OUTPUT
echo "----------------------------------------" >> $PERF_OUTPUT

# Eseguo il programma MPI su 2 nodi
echo "Running MPI program across 2 nodes with local perf monitoring (energy)..." >> $PERF_OUTPUT
perf stat -e power/energy-pkg/ -- mpiexec -n 2 ./mpi_vector_sum >> $PERF_OUTPUT 2>&1

# Fine test
echo "End Time: $(date)" >> $PERF_OUTPUT
echo "----------------------------------------" >> $PERF_OUTPUT
