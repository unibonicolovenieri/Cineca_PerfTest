#include "energy_meter.h"
#include <stdio.h>
#include <string.h>

static const char* zone_paths[] = {
    [ENERGY_PKG]  = "/sys/class/powercap/intel-rapl:0/energy_uj",
    [ENERGY_CORE] = "/sys/class/powercap/intel-rapl:0:0/energy_uj",
    [ENERGY_DRAM] = "/sys/class/powercap/intel-rapl:0:1/energy_uj",
    [ENERGY_PSYS] = "/sys/class/powercap/intel-rapl:1/energy_uj"
};

static long long read_energy_uj(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return -1;
    long long val = -1;
    fscanf(f, "%lld", &val);
    fclose(f);
    return val;
}

int energy_meter_init(energy_meter_t* m, int socket, energy_zone_t zone) {
    if (!m) return -1;
    m->socket = socket;
    m->zone = zone;
    m->start = 0;
    m->stop = 0;

    // Costruzione path dinamico
    switch (zone) {
        case ENERGY_PKG:
            snprintf(m->path, sizeof(m->path),
                     "/sys/class/powercap/intel-rapl:%d/energy_uj", socket);
            break;
        case ENERGY_CORE:
            snprintf(m->path, sizeof(m->path),
                     "/sys/class/powercap/intel-rapl:%d:0/energy_uj", socket);
            break;
        case ENERGY_DRAM:
            snprintf(m->path, sizeof(m->path),
                     "/sys/class/powercap/intel-rapl:%d:1/energy_uj", socket);
            break;
        case ENERGY_PSYS:
            snprintf(m->path, sizeof(m->path),
                     "/sys/class/powercap/intel-rapl:%d:2/energy_uj", socket);
            break;
        default:
            return -1;
    }

    return 0;
}


int energy_meter_start(energy_meter_t* m) {
    m->start = read_energy_uj(m->path);
    return m->start < 0 ? -1 : 0;
}

int energy_meter_stop(energy_meter_t* m) {
    m->stop = read_energy_uj(m->path);
    return m->stop < 0 ? -1 : 0;
}

double energy_meter_joules(energy_meter_t* m) {
    return (m->stop - m->start) / 1e6; // microjoule to joule
}
