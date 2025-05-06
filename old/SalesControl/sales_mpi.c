#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 256
#define MAX_BRAND 64

typedef struct {
    char brand[MAX_BRAND];
    int quantity;
} BrandSale;

void parse_line(char* line, int* year, char* brand, int* quantity) {
    char* token = strtok(line, ","); // Data
    *year = atoi(strtok(token, "-"));

    token = strtok(NULL, ","); // Marca
    strncpy(brand, token, MAX_BRAND);

    for (int i = 0; i < 2; i++) token = strtok(NULL, ","); // Salta Modello, Prezzo
    token = strtok(NULL, ","); // Quantità
    *quantity = atoi(token);
}

void analyze_sales(const char* filename, int target_year, BrandSale* result) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Errore apertura file");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    char line[MAX_LINE];
    BrandSale brands[20];
    int count = 0;

    fgets(line, MAX_LINE, file); // Skip intestazione

    while (fgets(line, MAX_LINE, file)) {
        int year, quantity;
        char brand[MAX_BRAND];
        parse_line(line, &year, brand, &quantity);

        if (year == target_year) {
            int found = 0;
            for (int i = 0; i < count; i++) {
                if (strcmp(brands[i].brand, brand) == 0) {
                    brands[i].quantity += quantity;
                    found = 1;
                    break;
                }
            }
            if (!found && count < 20) {
                strncpy(brands[count].brand, brand, MAX_BRAND);
                brands[count].quantity = quantity;
                count++;
            }
        }
    }

    fclose(file);

    strcpy(result->brand, "N/A");
    result->quantity = 0;
    for (int i = 0; i < count; i++) {
        if (brands[i].quantity > result->quantity) {
            strcpy(result->brand, brands[i].brand);
            result->quantity = brands[i].quantity;
        }
    }
}

int main(int argc, char* argv[]) {
    int rank, size;
    BrandSale result;
    const char* filename = "cpu_sales_log.csv";

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        analyze_sales(filename, 2013, &result);
        printf("[P%d] Anno 2013 - Top brand: %s (%d vendite)\n", rank, result.brand, result.quantity);
        MPI_Send(&result, sizeof(BrandSale), MPI_BYTE, 2, 0, MPI_COMM_WORLD);
    }
    else if (rank == 1) {
        analyze_sales(filename, 2024, &result);
        printf("[P%d] Anno 2024 - Top brand: %s (%d vendite)\n", rank, result.brand, result.quantity);
        MPI_Send(&result, sizeof(BrandSale), MPI_BYTE, 2, 1, MPI_COMM_WORLD);
    }
    else if (rank == 2) {
        BrandSale res2013, res2024, res2019;
        MPI_Status status;

        MPI_Recv(&res2013, sizeof(BrandSale), MPI_BYTE, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&res2024, sizeof(BrandSale), MPI_BYTE, 1, 1, MPI_COMM_WORLD, &status);

        if (strcmp(res2013.brand, res2024.brand) == 0) {
            analyze_sales(filename, 2019, &res2019);
            if (strcmp(res2019.brand, res2013.brand) == 0) {
                printf("[P%d] Anno 2019 - Stesso brand '%s' top (%d vendite)\n", rank, res2019.brand, res2019.quantity);
            } else {
                printf("[P%d] Anno 2019 - '%s' non è stato top (Top: %s)\n", rank, res2013.brand, res2019.brand);
            }
        } else {
            printf("[P%d] Top brand diversi → 2013: %s, 2024: %s → nessun confronto per 2019\n",
                   rank, res2013.brand, res2024.brand);
        }
    }

    MPI_Finalize();
    return 0;
}

