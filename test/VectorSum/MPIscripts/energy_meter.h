#ifndef ENERGY_METER_H
#define ENERGY_METER_H

typedef enum {
    ENERGY_PKG,
    ENERGY_CORE,
    ENERGY_DRAM,
    ENERGY_PSYS
} energy_zone_t;

typedef struct {
    int socket;
    energy_zone_t zone;
    long long start;
    long long stop;
    char path[256];
} energy_meter_t;

// Inizializzazione manuale (con socket specificato)
int energy_meter_init_manual(energy_meter_t* m, int socket, energy_zone_t zone);

// Inizializzazione automatica (detect socket corrente)
int energy_meter_init(energy_meter_t* m, energy_zone_t zone);

// Avvio e stop della misurazione
int energy_meter_start(energy_meter_t* m);
int energy_meter_stop(energy_meter_t* m);

// Calcolo dei joule
double energy_meter_joules(energy_meter_t* m);

// Utility: ottiene il socket corrente (dal core fisico in esecuzione)
int get_current_socket();

#endif
