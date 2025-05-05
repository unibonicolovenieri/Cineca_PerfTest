#!/bin/bash
#SBATCH --job-name=VectorSumMPI
#SBATCH --output=logs/output_%j.log
#SBATCH --ntasks-per-node=1    # 1 task per node
#SBATCH --nodes=2              # Usa 2 nodi
#SBATCH --time=00:10:00
#SBATCH --mail-type=BEGIN,END,FAIL
#SBATCH --mail-user=nicolo.venieri2@studio.unibo.it

# Creo cartelle logs/mpi_logs se non esistono
mkdir -p $HOME/cineca_perftest/test/VectorSum/logs/mpi_logs

# Carico i moduli necessari
module load gcc/12.2.0
module load openmpi/4.1.6--gcc--12.2.0

# Mi sposto nella cartella degli script
cd $HOME/cineca_perftest/test/VectorSum/MPIscripts

# File di output per perf
PERF_OUTPUT="$HOME/cineca_perftest/test/VectorSum/logs/mpi_logs/perf_output_%j.txt"
touch "$PERF_OUTPUT"

# Log generale per debug
GENERAL_LOG="$HOME/cineca_perftest/test/VectorSum/logs/mpi_logs/general_log_%j.txt"
touch "$GENERAL_LOG"

# Ottengo i nodi utilizzati
NODES=$(scontrol show job $SLURM_JOB_ID | grep "Nodes" | awk -F= '{print $2}')

# Intestazione log
echo "Job Name: $SLURM_JOB_NAME" >> "$GENERAL_LOG"
echo "Job ID: $SLURM_JOB_ID" >> "$GENERAL_LOG"
echo "Nodes Used: $NODES" >> "$GENERAL_LOG"
echo "Start Time: $(date)" >> "$GENERAL_LOG"
echo "----------------------------------------" >> "$GENERAL_LOG"

# Esegui il comando perf come contro-verifica
echo "Running perf for energy monitoring (pkg)..." >> "$PERF_OUTPUT"
perf stat -e power/energy-pkg/ -A srun ./mpi_energy_logger | tee -a "$PERF_OUTPUT"

# Lo rieseguo per il log generale
echo "Running MPI energy measurement program..." >> "$GENERAL_LOG"
srun ./mpi_energy_logger >> "$GENERAL_LOG"

# Fine test
echo "End Time: $(date)" >> "$GENERAL_LOG"
echo "----------------------------------------" >> "$GENERAL_LOG"
