#include "types.h"
#include <string.h>
#include <stdio.h>

int planet_building_level(const planet_t *p, const char *name) {
    if (strcmp(name, "metal_mine") == 0) return p->metal_mine_level;
    if (strcmp(name, "crystal_mine") == 0) return p->crystal_mine_level;
    if (strcmp(name, "gas_mine") == 0) return p->gas_mine_level;
    if (strcmp(name, "solar_plant") == 0) return p->solar_plant_level;
    if (strcmp(name, "fusion_reactor") == 0) return p->fusion_reactor_level;
    if (strcmp(name, "robotics") == 0) return p->robotics_level;
    if (strcmp(name, "nanite_factory") == 0) return p->nanite_factory_level;
    if (strcmp(name, "shipyard") == 0) return p->shipyard_level;
    if (strcmp(name, "research_lab") == 0) return p->research_lab_level;
    if (strcmp(name, "radar") == 0) return p->radar_level;
    if (strcmp(name, "alliance_depot") == 0) return p->alliance_depot_level;
    if (strcmp(name, "metal_storage") == 0) return p->metal_storage_level;
    if (strcmp(name, "crystal_storage") == 0) return p->crystal_storage_level;
    if (strcmp(name, "gas_storage") == 0) return p->gas_storage_level;
    if (strcmp(name, "moon_base") == 0) return p->moon_base_level;
    if (strcmp(name, "robotics_moon") == 0) return p->robotics_moon_level;
    if (strcmp(name, "shipyard_moon") == 0) return p->shipyard_moon_level;
    if (strcmp(name, "pioneer_lab") == 0) return p->pioneer_lab_level;
    if (strcmp(name, "eu_x_radar") == 0) return p->eu_x_radar_level;
    if (strcmp(name, "wormhole_generator") == 0) return p->wormhole_generator_level;
    if (strcmp(name, "galactonite_research_center") == 0) return p->galactonite_research_center_level;
    return 0;
}

void planet_set_building_level(planet_t *p, const char *name, int level) {
    if (strcmp(name, "metal_mine") == 0) { p->metal_mine_level = level; return; }
    if (strcmp(name, "crystal_mine") == 0) { p->crystal_mine_level = level; return; }
    if (strcmp(name, "gas_mine") == 0) { p->gas_mine_level = level; return; }
    if (strcmp(name, "solar_plant") == 0) { p->solar_plant_level = level; return; }
    if (strcmp(name, "fusion_reactor") == 0) { p->fusion_reactor_level = level; return; }
    if (strcmp(name, "robotics") == 0) { p->robotics_level = level; return; }
    if (strcmp(name, "nanite_factory") == 0) { p->nanite_factory_level = level; return; }
    if (strcmp(name, "shipyard") == 0) { p->shipyard_level = level; return; }
    if (strcmp(name, "research_lab") == 0) { p->research_lab_level = level; return; }
    if (strcmp(name, "radar") == 0) { p->radar_level = level; return; }
    if (strcmp(name, "alliance_depot") == 0) { p->alliance_depot_level = level; return; }
    if (strcmp(name, "metal_storage") == 0) { p->metal_storage_level = level; return; }
    if (strcmp(name, "crystal_storage") == 0) { p->crystal_storage_level = level; return; }
    if (strcmp(name, "gas_storage") == 0) { p->gas_storage_level = level; return; }
    if (strcmp(name, "moon_base") == 0) { p->moon_base_level = level; return; }
    if (strcmp(name, "robotics_moon") == 0) { p->robotics_moon_level = level; return; }
    if (strcmp(name, "shipyard_moon") == 0) { p->shipyard_moon_level = level; return; }
    if (strcmp(name, "pioneer_lab") == 0) { p->pioneer_lab_level = level; return; }
    if (strcmp(name, "eu_x_radar") == 0) { p->eu_x_radar_level = level; return; }
    if (strcmp(name, "wormhole_generator") == 0) { p->wormhole_generator_level = level; return; }
    if (strcmp(name, "galactonite_research_center") == 0) { p->galactonite_research_center_level = level; return; }
}

static int *ship_field_ptr(fleet_ships_t *fs, const char *type) {
    if (strcmp(type, "light_fighter") == 0) return &fs->light_fighter;
    if (strcmp(type, "heavy_fighter") == 0) return &fs->heavy_fighter;
    if (strcmp(type, "cruiser") == 0) return &fs->cruiser;
    if (strcmp(type, "battleship") == 0) return &fs->battleship;
    if (strcmp(type, "dreadnought") == 0) return &fs->dreadnought;
    if (strcmp(type, "destroyer") == 0) return &fs->destroyer;
    if (strcmp(type, "bomber") == 0) return &fs->bomber;
    if (strcmp(type, "interceptor") == 0) return &fs->interceptor;
    if (strcmp(type, "iron_behemoth") == 0) return &fs->iron_behemoth;
    if (strcmp(type, "devourer_interceptor") == 0) return &fs->devourer_interceptor;
    if (strcmp(type, "titanenergetic_frigate") == 0) return &fs->titanenergetic_frigate;
    if (strcmp(type, "annihilator") == 0) return &fs->annihilator;
    if (strcmp(type, "small_cargo") == 0) return &fs->small_cargo;
    if (strcmp(type, "large_cargo") == 0) return &fs->large_cargo;
    if (strcmp(type, "recycler") == 0) return &fs->recycler;
    if (strcmp(type, "colony_ship") == 0) return &fs->colony_ship;
    if (strcmp(type, "probe") == 0) return &fs->probe;
    if (strcmp(type, "solar_satellite") == 0) return &fs->solar_satellite;
    return NULL;
}

static const int *ship_field_ptr_const(const fleet_ships_t *fs, const char *type) {
    return ship_field_ptr((fleet_ships_t *)fs, type);
}

fleet_ships_t fleet_ships_zero(void) {
    fleet_ships_t fs;
    memset(&fs, 0, sizeof(fs));
    return fs;
}

int fleet_ships_total(const fleet_ships_t *fs) {
    int n = 0;
    n += fs->light_fighter;
    n += fs->heavy_fighter;
    n += fs->cruiser;
    n += fs->battleship;
    n += fs->dreadnought;
    n += fs->destroyer;
    n += fs->bomber;
    n += fs->interceptor;
    n += fs->iron_behemoth;
    n += fs->devourer_interceptor;
    n += fs->titanenergetic_frigate;
    n += fs->annihilator;
    n += fs->small_cargo;
    n += fs->large_cargo;
    n += fs->recycler;
    n += fs->colony_ship;
    n += fs->probe;
    n += fs->solar_satellite;
    return n;
}

int fleet_ships_get(const fleet_ships_t *fs, const char *type) {
    const int *p = ship_field_ptr_const(fs, type);
    return p ? *p : 0;
}

void fleet_ships_set(fleet_ships_t *fs, const char *type, int count) {
    int *p = ship_field_ptr(fs, type);
    if (p) *p = count;
}

void fleet_ships_add(fleet_ships_t *fs, const char *type, int count) {
    int *p = ship_field_ptr(fs, type);
    if (p) *p += count;
}

void fleet_ships_sub(fleet_ships_t *fs, const char *type, int count) {
    int *p = ship_field_ptr(fs, type);
    if (p) { *p -= count; if (*p < 0) *p = 0; }
}

int fleet_ships_has_enough(const fleet_ships_t *fs, const fleet_ships_t *other) {
    return fs->light_fighter >= other->light_fighter &&
           fs->heavy_fighter >= other->heavy_fighter &&
           fs->cruiser >= other->cruiser &&
           fs->battleship >= other->battleship &&
           fs->dreadnought >= other->dreadnought &&
           fs->destroyer >= other->destroyer &&
           fs->bomber >= other->bomber &&
           fs->interceptor >= other->interceptor &&
           fs->iron_behemoth >= other->iron_behemoth &&
           fs->devourer_interceptor >= other->devourer_interceptor &&
           fs->titanenergetic_frigate >= other->titanenergetic_frigate &&
           fs->annihilator >= other->annihilator &&
           fs->small_cargo >= other->small_cargo &&
           fs->large_cargo >= other->large_cargo &&
           fs->recycler >= other->recycler &&
           fs->colony_ship >= other->colony_ship &&
           fs->probe >= other->probe &&
           fs->solar_satellite >= other->solar_satellite;
}

void fleet_ships_sum(fleet_ships_t *fs, const fleet_ships_t *other) {
    fs->light_fighter += other->light_fighter;
    fs->heavy_fighter += other->heavy_fighter;
    fs->cruiser += other->cruiser;
    fs->battleship += other->battleship;
    fs->dreadnought += other->dreadnought;
    fs->destroyer += other->destroyer;
    fs->bomber += other->bomber;
    fs->interceptor += other->interceptor;
    fs->iron_behemoth += other->iron_behemoth;
    fs->devourer_interceptor += other->devourer_interceptor;
    fs->titanenergetic_frigate += other->titanenergetic_frigate;
    fs->annihilator += other->annihilator;
    fs->small_cargo += other->small_cargo;
    fs->large_cargo += other->large_cargo;
    fs->recycler += other->recycler;
    fs->colony_ship += other->colony_ship;
    fs->probe += other->probe;
    fs->solar_satellite += other->solar_satellite;
}

int64_t fleet_ships_cargo_capacity(const fleet_ships_t *fs) {
    int64_t total = 0;
    total += (int64_t)fs->small_cargo * 5000;
    total += (int64_t)fs->large_cargo * 25000;
    total += (int64_t)fs->recycler * 20000;
    total += (int64_t)fs->colony_ship * 7500;
    total += (int64_t)fs->probe * 5;
    total += (int64_t)fs->solar_satellite * 0;
    total += (int64_t)fs->light_fighter * 50;
    total += (int64_t)fs->heavy_fighter * 100;
    total += (int64_t)fs->cruiser * 800;
    total += (int64_t)fs->battleship * 1500;
    total += (int64_t)fs->dreadnought * 3000;
    total += (int64_t)fs->destroyer * 2000;
    total += (int64_t)fs->bomber * 750;
    total += (int64_t)fs->interceptor * 400;
    total += (int64_t)fs->iron_behemoth * 0;
    total += (int64_t)fs->devourer_interceptor * 0;
    total += (int64_t)fs->titanenergetic_frigate * 0;
    total += (int64_t)fs->annihilator * 0;
    return total;
}

void fleet_ships_each(const fleet_ships_t *fs, void (*fn)(const char *type, int count, void *ctx), void *ctx) {
    if (fs->light_fighter > 0) fn("light_fighter", fs->light_fighter, ctx);
    if (fs->heavy_fighter > 0) fn("heavy_fighter", fs->heavy_fighter, ctx);
    if (fs->cruiser > 0) fn("cruiser", fs->cruiser, ctx);
    if (fs->battleship > 0) fn("battleship", fs->battleship, ctx);
    if (fs->dreadnought > 0) fn("dreadnought", fs->dreadnought, ctx);
    if (fs->destroyer > 0) fn("destroyer", fs->destroyer, ctx);
    if (fs->bomber > 0) fn("bomber", fs->bomber, ctx);
    if (fs->interceptor > 0) fn("interceptor", fs->interceptor, ctx);
    if (fs->iron_behemoth > 0) fn("iron_behemoth", fs->iron_behemoth, ctx);
    if (fs->devourer_interceptor > 0) fn("devourer_interceptor", fs->devourer_interceptor, ctx);
    if (fs->titanenergetic_frigate > 0) fn("titanenergetic_frigate", fs->titanenergetic_frigate, ctx);
    if (fs->annihilator > 0) fn("annihilator", fs->annihilator, ctx);
    if (fs->small_cargo > 0) fn("small_cargo", fs->small_cargo, ctx);
    if (fs->large_cargo > 0) fn("large_cargo", fs->large_cargo, ctx);
    if (fs->recycler > 0) fn("recycler", fs->recycler, ctx);
    if (fs->colony_ship > 0) fn("colony_ship", fs->colony_ship, ctx);
    if (fs->probe > 0) fn("probe", fs->probe, ctx);
    if (fs->solar_satellite > 0) fn("solar_satellite", fs->solar_satellite, ctx);
}
