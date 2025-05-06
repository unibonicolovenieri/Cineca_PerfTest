#!/bin/bash
#########################################
# SLURM script per analisi vendite MPI
#########################################

#SBATCH --job-name=sales_mpi_analysis
#SBATCH --time=01:00:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=3
#SBATCH --cpus-per-task=1
#SBATCH --mem=10000
#SBATCH --partition=boost_usr_prod
#SBATCH --qos=normal
#SBATCH --account=nvenieri
#SBATCH --output=dcgp_usr_prod 
#SBATCH --error=sales_job.err

#SBATCH --mail-type=BEGIN,END,FAIL
#SBATCH --mail-user=nicolo.venieri2@studio.unibo.it

# === MODULI ===
module load intel
module load intelmpi

# === VARIABILI SLURM ===
export SRUN_CPUS_PER_TASK=$SLURM_CPUS_PER_TASK
export OMP_NUM_THREADS=1

# === SETUP LOGGING ===
LOGDIR="$SLURM_SUBMIT_DIR/logs"
mkdir -p "$LOGDIR"

LOGFILE="$LOGDIR/job_${SLURM_JOB_ID}.log"

echo "=== Job Info ==="                             > "$LOGFILE"
echo "Timestamp     : $(date)"                    >> "$LOGFILE"
echo "Job ID        : $SLURM_JOB_ID"              >> "$LOGFILE"
echo "Job Name      : $SLURM_JOB_NAME"            >> "$LOGFILE"
echo "Nodes         : $SLURM_JOB_NUM_NODES"       >> "$LOGFILE"
echo "Tasks/Node    : $SLURM_NTASKS_PER_NODE"     >> "$LOGFILE"
echo "CPUs/Task     : $SLURM_CPUS_PER_TASK"       >> "$LOGFILE"
echo "Partition     : $SLURM_JOB_PARTITION"       >> "$LOGFILE"
echo "QoS           : $SLURM_QOS"                 >> "$LOGFILE"
echo "Account       : $SLURM_JOB_ACCOUNT"         >> "$LOGFILE"
echo "=========================="                 >> "$LOGFILE"

# === FUNZIONE PER ESEGUIRE E LOGGARE COMANDI ===
run_and_log() {
  echo "[RUN] $(date +"%F %T") » $*" >> "$LOGFILE"
  eval "$@" >> "$LOGFILE" 2>&1
  echo "---" >> "$LOGFILE"
}

# === COMANDI DA ESEGUIRE ===
run_and_log srun ./sales_mpi
# Esempio aggiuntivo (decommenta se vuoi usarlo)
run_and_log srun perf stat -e cache-misses ./sales_mpi
run_and_log echo "Analisi completata con successo"

