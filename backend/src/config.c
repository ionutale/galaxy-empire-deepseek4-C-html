#include "config.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

const char *ship_type_names[NUM_SHIP_TYPES] = {
    "light_fighter", "heavy_fighter", "cruiser", "battleship",
    "dreadnought", "destroyer", "bomber", "interceptor",
    "iron_behemoth", "devourer_interceptor", "titanenergetic_frigate", "annihilator",
    "small_cargo", "large_cargo", "recycler", "colony_ship", "probe", "solar_satellite"
};

const char *defense_type_names[NUM_DEFENSE_TYPES] = {
    "rocket_launcher", "light_laser", "heavy_laser", "ion_cannon",
    "mk2_cannon", "plasma_cannon", "proton_cannon",
    "small_shield_dome", "large_shield_dome"
};

const ship_stats_t ship_stats_table[NUM_SHIP_TYPES] = {
    {"light_fighter", 3000, 1000, 0, 10, 10, 50, 12500, 50, 20},
    {"heavy_fighter", 6000, 4000, 0, 25, 25, 150, 10000, 100, 75},
    {"cruiser", 20000, 7000, 2000, 100, 50, 400, 15000, 800, 300},
    {"battleship", 45000, 15000, 0, 200, 200, 1000, 10000, 1500, 500},
    {"dreadnought", 50000, 25000, 15000, 500, 200, 1500, 10000, 3000, 1000},
    {"destroyer", 60000, 50000, 15000, 500, 500, 2000, 9000, 2000, 2000},
    {"bomber", 50000, 25000, 15000, 500, 500, 1000, 4000, 750, 1000},
    {"interceptor", 30000, 40000, 15000, 400, 400, 1, 12000, 400, 500},
    {"iron_behemoth", 1000000, 500000, 100000, 10000, 5000, 5000, 8000, 0, 5000},
    {"devourer_interceptor", 200000, 200000, 150000, 5000, 5000, 5000, 8000, 0, 5000},
    {"titanenergetic_frigate", 300000, 200000, 150000, 1000, 1000, 1, 9000, 0, 1000},
    {"annihilator", 50000, 50000, 30000, 1000, 1, 2000, 5000, 0, 2000},
    {"small_cargo", 2000, 2000, 0, 5, 5, 5, 12500, 5000, 10},
    {"large_cargo", 6000, 6000, 0, 10, 10, 5, 11000, 25000, 50},
    {"recycler", 10000, 6000, 2000, 20, 10, 1, 4000, 20000, 300},
    {"colony_ship", 10000, 20000, 10000, 100, 100, 50, 2500, 7500, 1000},
    {"probe", 0, 1000, 0, 1, 1, 1, 100000000, 5, 1},
    {"solar_satellite", 0, 2000, 500, 1, 1, 1, 0, 0, 0}
};

const ship_stats_t defense_stats_table[NUM_DEFENSE_TYPES] = {
    {"rocket_launcher", 2000, 0, 0, 20, 20, 80, 0, 0, 0},
    {"light_laser", 1500, 500, 0, 25, 25, 100, 0, 0, 0},
    {"heavy_laser", 6000, 2000, 0, 100, 100, 250, 0, 0, 0},
    {"ion_cannon", 2000, 6000, 0, 500, 100, 150, 0, 0, 0},
    {"mk2_cannon", 15000, 11000, 0, 300, 300, 500, 0, 0, 0},
    {"plasma_cannon", 50000, 50000, 30000, 600, 300, 2000, 0, 0, 0},
    {"proton_cannon", 75000, 85000, 30000, 2000, 500, 3500, 0, 0, 0},
    {"small_shield_dome", 10000, 10000, 0, 2000, 2000, 1, 0, 0, 0},
    {"large_shield_dome", 50000, 50000, 0, 10000, 10000, 1, 0, 0, 0}
};

const building_config_t building_configs[NUM_BUILDINGS] = {
    {"metal_mine", 60, 15, 0, 1.5, 17.76721463, 0.406103736, 0},
    {"crystal_mine", 48, 24, 0, 1.5, 17.76721463, 0.406103736, 0},
    {"gas_mine", 75, 30, 0, 1.5, 17.76721463, 0.406103736, 0},
    {"solar_plant", 75, 30, 0, 1.5, 17.76721463, 0.406103736, 0},
    {"fusion_reactor", 900, 360, 180, 1.5, 17.76721463, 0.406103736, 0},
    {"robotics", 400, 120, 200, 2.0, 17.76721463, 0.406103736, 0},
    {"nanite_factory", 1000000, 500000, 100000, 2.0, 17.76721463, 0.406103736, 0},
    {"shipyard", 400, 200, 100, 2.0, 17.76721463, 0.406103736, 0},
    {"research_lab", 200, 400, 200, 2.0, 17.76721463, 0.406103736, 0},
    {"radar", 200, 1000, 200, 2.0, 17.76721463, 0.406103736, 0},
    {"alliance_depot", 20000, 40000, 0, 2.0, 17.76721463, 0.406103736, 0},
    {"metal_storage", 1000, 0, 0, 2.0, 17.76721463, 0.406103736, 0},
    {"crystal_storage", 1000, 500, 0, 2.0, 17.76721463, 0.406103736, 0},
    {"gas_storage", 1000, 1000, 0, 2.0, 17.76721463, 0.406103736, 0},
    {"moon_base", 20, 10, 0, 2.0, 17.76721463, 0.406103736, 1},
    {"robotics_moon", 400, 120, 200, 2.0, 17.76721463, 0.406103736, 1},
    {"shipyard_moon", 400, 200, 100, 2.0, 17.76721463, 0.406103736, 1},
    {"pioneer_lab", 20000, 40000, 20000, 2.0, 17.76721463, 0.406103736, 1},
    {"eu_x_radar", 50000, 100000, 50000, 2.0, 17.76721463, 0.406103736, 1},
    {"wormhole_generator", 500000, 1000000, 500000, 2.0, 17.76721463, 0.406103736, 1},
    {"galactonite_research_center", 200000, 400000, 200000, 2.0, 17.76721463, 0.406103736, 0}
};

const char *tech_names[NUM_TECHS] = {
    "energy_tech", "laser_tech", "ion_tech", "hyperspace_tech",
    "plasma_tech", "ultra_temperature_tech", "anti_gravity_tech",
    "weapons_tech", "shielding_tech", "strength_tech",
    "combustion_drive", "impulse_drive", "hyperspace_drive",
    "astrophysics", "computer_tech", "espionage_tech"
};

const tech_config_t tech_configs[NUM_TECHS] = {
    {"energy_tech", 800, 2.0, 0, 1, NULL, 0},
    {"laser_tech", 200, 2.0, 0, 1, NULL, 0},
    {"ion_tech", 1000, 2.0, 0, 4, NULL, 0},
    {"hyperspace_tech", 4000, 2.0, 0, 7, NULL, 0},
    {"plasma_tech", 2000, 2.0, 0, 4, NULL, 0},
    {"ultra_temperature_tech", 40000, 1.6, 0, 8, NULL, 0},
    {"anti_gravity_tech", 120000, 1.6, 0, 8, NULL, 0},
    {"weapons_tech", 800, 2.0, 0, 4, NULL, 0},
    {"shielding_tech", 200, 2.0, 0, 3, NULL, 0},
    {"strength_tech", 200, 2.0, 0, 3, NULL, 0},
    {"combustion_drive", 400, 2.0, 0, 1, NULL, 0},
    {"impulse_drive", 2000, 2.0, 0, 2, NULL, 0},
    {"hyperspace_drive", 10000, 2.0, 0, 7, NULL, 0},
    {"astrophysics", 4000, 1.7, 0, 4, NULL, 0},
    {"computer_tech", 400, 2.0, 0, 1, NULL, 0},
    {"espionage_tech", 200, 2.0, 0, 3, NULL, 0}
};

const npc_fleet_config_t npc_fleets[NUM_NPC_FLEETS] = {
    {"the_purged", {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 1000, 500, 0},
    {"galaxy_pirates", {500,200,50,0,0,0,0,0,0,0,0,0,200,100,50,0,0,0}, 5000, 2500, 1000},
    {"pollution_origins", {0,0,0,200,0,50,0,0,0,0,0,0,0,0,0,0,0,0}, 10000, 5000, 2500},
    {"bloody_palace", {0,0,100,50,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 20000, 10000, 5000},
    {"alien_brood", {0,0,0,50,20,0,0,0,0,0,0,0,0,0,0,0,0,0}, 50000, 25000, 10000},
    {"dwarf_star_fortress", {0,0,2000,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0}, 0, 0, 0}
};

const char *npc_position_keys[5] = {"the_purged", "galaxy_pirates", "pollution_origins", "bloody_palace", "alien_brood"};

int get_ship_idx(const char *type) {
    for (int i = 0; i < NUM_SHIP_TYPES; i++)
        if (strcmp(ship_type_names[i], type) == 0) return i;
    return -1;
}

int get_defense_idx(const char *type) {
    for (int i = 0; i < NUM_DEFENSE_TYPES; i++)
        if (strcmp(defense_type_names[i], type) == 0) return i;
    return -1;
}

int get_building_idx(const char *name) {
    for (int i = 0; i < NUM_BUILDINGS; i++)
        if (strcmp(building_configs[i].name, name) == 0) return i;
    return -1;
}

int get_tech_idx(const char *name) {
    for (int i = 0; i < NUM_TECHS; i++)
        if (strcmp(tech_configs[i].name, name) == 0) return i;
    return -1;
}

int get_npc_idx(int position) {
    if (position < 17 || position > 21) return -1;
    return position - 17;
}

const ship_stats_t *get_ship_stats(const char *type) {
    int i = get_ship_idx(type);
    if (i >= 0) return &ship_stats_table[i];
    i = get_defense_idx(type);
    if (i >= 0) return &defense_stats_table[i];
    return NULL;
}

int64_t get_building_cost(int building_idx, int level) {
    if (building_idx < 0 || building_idx >= NUM_BUILDINGS) return 0;
    const building_config_t *cfg = &building_configs[building_idx];
    return (int64_t)(cfg->base_metal * pow(cfg->multiplier, level - 1));
}

int get_build_time(int building_idx, int level, int robotics_level, int nanite_level) {
    if (building_idx < 0 || building_idx >= NUM_BUILDINGS) return 0;
    const building_config_t *cfg = &building_configs[building_idx];
    double t = (cfg->formula_a / (robotics_level + 1)) * exp(cfg->formula_b * level);
    if (nanite_level > 0) t = t / pow(2.0, nanite_level);
    return (int)t;
}

int64_t get_tech_cost(int tech_idx, int level) {
    if (tech_idx < 0 || tech_idx >= NUM_TECHS) return 0;
    const tech_config_t *cfg = &tech_configs[tech_idx];
    return (int64_t)(cfg->base_cost * pow(cfg->multiplier, level - 1));
}

int get_tech_duration(int tech_idx, int level, int lab_level) {
    if (tech_idx < 0 || tech_idx >= NUM_TECHS) return 0;
    int64_t cost = get_tech_cost(tech_idx, level);
    return (int)(cost / (lab_level + 1));
}

int64_t get_unit_cost_metal(const char *type, int is_defense) {
    const ship_stats_t *s = get_ship_stats(type);
    return s ? s->metal : 0;
}

int64_t get_unit_cost_crystal(const char *type, int is_defense) {
    const ship_stats_t *s = get_ship_stats(type);
    return s ? s->crystal : 0;
}

int get_production_time(const char *type, int is_defense, int shipyard_level, int nanite_level) {
    const ship_stats_t *s = get_ship_stats(type);
    if (!s) return 0;
    int64_t total_cost = s->metal + s->crystal;
    double t = (double)total_cost / (11132.0 * (shipyard_level + 1));
    if (nanite_level > 0) t = t / pow(2.0, nanite_level);
    return (int)(t * 3600);
}
