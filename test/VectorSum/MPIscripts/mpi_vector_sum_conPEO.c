#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>

#define PERF_EVENT_OPEN_SYSCALL_NR __NR_perf_event_open

#define VECTOR_SIZE 50

void read_vector(const char* filename, float* vector) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Errore nell'aprire il file");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    for (int i = 0; i < VECTOR_SIZE; i++) {
        fscanf(file, "%f", &vector[i]);
    }

    fclose(file);
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Assicurati che ci siano esattamente 2 processi
    if (size != 2) {
        if (rank == 0) {
            printf("Questo programma richiede esattamente 2 processi MPI.\n");
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Percorsi dei file
    char vector1_path[] = "../generation/vector1.txt";
    char vector2_path[] = "../generation/vector2.txt";

    // Vettori per ogni rank
    float vector1[VECTOR_SIZE], vector2[VECTOR_SIZE];
    float local_vector[VECTOR_SIZE / 2];
//preambolo
    int type = 0x17; // 23 in hex
    long config = 0x02; // event code in hex

    // Set up the perf_event_attr struct
    struct perf_event_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.type = type;
    attr.size = sizeof(struct perf_event_attr);
    attr.config = config;

    // Open the event
    long fd = syscall(PERF_EVENT_OPEN_SYSCALL_NR, &attr, -1, 0, -1, 0);
    if (fd == -1) {
        perror("perf_event_open");
        return 1;
    }

    // Read initial counter value
    long int counter_before;
    if (read(fd, &counter_before, sizeof(long int)) != sizeof(long int)) {
        perror("read");
        close(fd);
        return 1;
    }
//fine preambolo
    if (rank == 0) {
        // Rank 0 legge i vettori
        read_vector(vector1_path, vector1);
        read_vector(vector2_path, vector2);

        // Rank 0 divide i dati
        for (int i = 0; i < VECTOR_SIZE / 2; i++) {
            local_vector[i] = vector1[i] + vector2[i];  // Prima metà
        }

        // Manda la seconda metà dei vettori a rank 1
        MPI_Send(&vector1[VECTOR_SIZE / 2], VECTOR_SIZE / 2, MPI_FLOAT, 1, 0, MPI_COMM_WORLD);
        MPI_Send(&vector2[VECTOR_SIZE / 2], VECTOR_SIZE / 2, MPI_FLOAT, 1, 1, MPI_COMM_WORLD);

    } else if (rank == 1) {
        // Rank 1 riceve la seconda metà dei vettori
        MPI_Recv(&vector1[VECTOR_SIZE / 2], VECTOR_SIZE / 2, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&vector2[VECTOR_SIZE / 2], VECTOR_SIZE / 2, MPI_FLOAT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Rank 1 somma la seconda metà
        for (int i = VECTOR_SIZE / 2; i < VECTOR_SIZE; i++) {
            local_vector[i - VECTOR_SIZE / 2] = vector1[i] + vector2[i];
        }
    }

    // Ogni rank calcola la somma locale
    float local_sum = 0.0;
    for (int i = 0; i < VECTOR_SIZE / 2; i++) {
        local_sum += local_vector[i];
    }

    // Rank 0 raccoglie il risultato da rank 1
    if (rank == 0) {
        float global_sum = local_sum;
        float sum_from_rank_1;

        // Riceve la somma parziale da rank 1
        MPI_Recv(&sum_from_rank_1, 1, MPI_FLOAT, 1, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        global_sum += sum_from_rank_1;

        printf("Somma totale dei vettori: %.4f\n", global_sum);
    } else if (rank == 1) {
        // Rank 1 manda la somma parziale a rank 0
        MPI_Send(&local_sum, 1, MPI_FLOAT, 0, 2, MPI_COMM_WORLD);
    }
//epilogo
    // Read the final counter value
    long int counter_after;
    if (read(fd, &counter_after, sizeof(long int)) != sizeof(long int)) {
        perror("read");
        close(fd);
        return 1;
    }

    // Calculate energy usage
    double scale = 2.3283064365386962890625e-10; // from energy-cores.scale
    double energy = (counter_after - counter_before) * scale;
    printf("Energy usage: %.9f Joules\n", energy);

    // Close the event
    close(fd);
    return 0;
//fine epilogo

    MPI_Finalize();
    return 0;
}
