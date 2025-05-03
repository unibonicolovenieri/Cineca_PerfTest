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


int energy_meter_init(energy_meter_t* m, energy_zone_t zone);
int energy_meter_start(energy_meter_t* m);
int energy_meter_stop(energy_meter_t* m);
double energy_meter_joules(energy_meter_t* m);

#endif
