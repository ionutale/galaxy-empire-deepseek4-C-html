#ifndef CONFIG_H
#define CONFIG_H

#include "domain/types.h"
#include <stdint.h>

#define NUM_SHIP_TYPES 18
#define NUM_DEFENSE_TYPES 9
#define NUM_BUILDINGS 21
#define NUM_TECHS 16
#define NUM_NPC_FLEETS 6

extern const char *ship_type_names[NUM_SHIP_TYPES];
extern const char *defense_type_names[NUM_DEFENSE_TYPES];
extern const char *building_names[NUM_BUILDINGS];
extern const char *tech_names[NUM_TECHS];

extern const ship_stats_t ship_stats_table[NUM_SHIP_TYPES];
extern const ship_stats_t defense_stats_table[NUM_DEFENSE_TYPES];

typedef struct {
    const char *name;
    int64_t base_metal;
    int64_t base_crystal;
    int64_t base_gas;
    double multiplier;
    double formula_a;
    double formula_b;
    int is_moon_building;
} building_config_t;

extern const building_config_t building_configs[NUM_BUILDINGS];

typedef struct {
    const char *name;
    int64_t base_cost;
    double multiplier;
    int max_level;
    int lab_level_required;
    const char **prereq_techs;
    int prereq_count;
} tech_config_t;

extern const tech_config_t tech_configs[NUM_TECHS];

typedef struct {
    const char *key;
    fleet_ships_t ships;
    int64_t metal;
    int64_t crystal;
    int64_t gas;
} npc_fleet_config_t;

extern const npc_fleet_config_t npc_fleets[NUM_NPC_FLEETS];
extern const char *npc_position_keys[5];

int64_t get_building_cost(int building_idx, int level);
int get_build_time(int building_idx, int level, int robotics_level, int nanite_level);
int64_t get_tech_cost(int tech_idx, int level);
int get_tech_duration(int tech_idx, int level, int lab_level);
int64_t get_unit_cost_metal(const char *type, int is_defense);
int64_t get_unit_cost_crystal(const char *type, int is_defense);
int get_production_time(const char *type, int is_defense, int shipyard_level, int nanite_level);
int get_ship_idx(const char *type);
int get_defense_idx(const char *type);
int get_building_idx(const char *name);
int get_tech_idx(const char *name);
int get_npc_idx(int position);
const ship_stats_t *get_ship_stats(const char *type);

#endif
