#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include <string.h>
#include "energy_meter.h"

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    char hostname[128];
    gethostname(hostname, sizeof(hostname));

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    energy_meter_t meter;
    if (energy_meter_init(&meter, ENERGY_PKG) != 0) {
        fprintf(stderr, "[%s][Rank %d] Errore init energy_meter\n", hostname, rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    char log_path[512];
    snprintf(log_path, sizeof(log_path),
             "%s/cineca_perftest/test/VectorSum/logs/mpi_logs/rank_%d.log",
             getenv("HOME"), rank);

    FILE* log_file = fopen(log_path, "w");
    if (!log_file) {
        fprintf(stderr, "[%s][Rank %d] Errore apertura file log\n", hostname, rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    fprintf(log_file, "[Rank %d] Host: %s - Socket: %d\n", rank, hostname, meter.socket);
    fprintf(log_file, "Inizio misurazione energia...\n");

    if (energy_meter_start(&meter) != 0) {
        fprintf(log_file, "Errore in energy_meter_start()\n");
        fclose(log_file);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    double x = 0.0;
    for (int i = 0; i < 1e8; i++) {
        x += i * 0.00001;
    }

    if (energy_meter_stop(&meter) != 0) {
        fprintf(log_file, "Errore in energy_meter_stop()\n");
        fclose(log_file);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    double consumed = energy_meter_joules(&meter);
    fprintf(log_file, "Energia consumata: %.6f Joule\n", consumed);
    fprintf(log_file, "Fine test\n");

    fclose(log_file);
    MPI_Finalize();
    return 0;
}
