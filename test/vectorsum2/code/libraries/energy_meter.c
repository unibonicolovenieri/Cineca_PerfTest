#include "energy_meter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sched.h>

static long long read_energy_uj(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return -1;
    long long val = -1;
    fscanf(f, "%lld", &val);
    fclose(f);
    return val;
}

int get_current_socket() {
    int cpu = sched_getcpu();
    char path[256];
    snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/topology/physical_package_id", cpu);

    FILE* f = fopen(path, "r");
    if (!f) return -1;

    int socket = -1;
    fscanf(f, "%d", &socket);
    fclose(f);
    return socket;
}

int energy_meter_init_manual(energy_meter_t* m, int socket, energy_zone_t zone) {
    if (!m) return -1;
    m->socket = socket;
    m->zone = zone;
    m->start = 0;
    m->stop = 0;

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

int energy_meter_init(energy_meter_t* m, energy_zone_t zone) {
    int socket = get_current_socket();
    if (socket < 0) return -1;
    return energy_meter_init_manual(m, socket, zone);
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
    return (m->stop - m->start) / 1e6;
}
