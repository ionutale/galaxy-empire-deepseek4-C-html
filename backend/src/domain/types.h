#ifndef DOMAIN_TYPES_H
#define DOMAIN_TYPES_H

#include <stdint.h>
#include <time.h>

#define MAX_USERNAME_LEN 64
#define MAX_PASSWORD_LEN 255
#define MAX_HASH_LEN 255

typedef int64_t entity_id_t;

typedef enum {
    MT_METAL = 0, MT_CRYSTAL, MT_GAS, MT_DARK_MATTER, MT_CREDITS
} mission_type_t;

typedef struct {
    entity_id_t id;
    int server_id;
    int galaxy_num;
    int system_num;
    int position_num;
    int is_moon;
} coordinate_t;

typedef struct {
    entity_id_t id;
    entity_id_t user_id;
    entity_id_t coordinate_id;
    int planet_size;
    int temperature;
    int metal_mine_level;
    int crystal_mine_level;
    int gas_mine_level;
    int solar_plant_level;
    int fusion_reactor_level;
    int robotics_level;
    int nanite_factory_level;
    int shipyard_level;
    int research_lab_level;
    int radar_level;
    int alliance_depot_level;
    int metal_storage_level;
    int crystal_storage_level;
    int gas_storage_level;
    int moon_base_level;
    int robotics_moon_level;
    int shipyard_moon_level;
    int pioneer_lab_level;
    int eu_x_radar_level;
    int wormhole_generator_level;
    int galactonite_research_center_level;
    double metal;
    double crystal;
    double gas;
    double energy;
    time_t last_resource_update;
    time_t last_updated;
    entity_id_t construction_id;
    char const_building[32];
    int const_target_level;
    time_t construction_end;
    entity_id_t production_id;
    char prod_unit_type[32];
    int prod_quantity;
    time_t production_end;
} planet_t;

typedef struct {
    entity_id_t id;
    char building[32];
    int target_level;
} queue_item_t;

typedef struct {
    entity_id_t id;
    char unit_type[32];
    int quantity;
} prod_queue_item_t;

typedef struct {
    entity_id_t id;
    entity_id_t user_id;
    int energy_tech;
    int laser_tech;
    int ion_tech;
    int hyperspace_tech;
    int plasma_tech;
    int ultra_temperature_tech;
    int anti_gravity_tech;
    int weapons_tech;
    int shielding_tech;
    int strength_tech;
    int combustion_drive;
    int impulse_drive;
    int hyperspace_drive;
    int astrophysics;
    int computer_tech;
    int espionage_tech;
    char queued_tech[32];
    time_t research_start;
    time_t research_end;
} research_t;

typedef struct {
    entity_id_t id;
    char tech[32];
} research_queue_item_t;

typedef struct {
    int light_fighter;
    int heavy_fighter;
    int cruiser;
    int battleship;
    int dreadnought;
    int destroyer;
    int bomber;
    int interceptor;
    int iron_behemoth;
    int devourer_interceptor;
    int titanenergetic_frigate;
    int annihilator;
    int small_cargo;
    int large_cargo;
    int recycler;
    int colony_ship;
    int probe;
    int solar_satellite;
} fleet_ships_t;

int planet_building_level(const planet_t *p, const char *building_name);
void planet_set_building_level(planet_t *p, const char *building_name, int level);

fleet_ships_t fleet_ships_zero(void);
int fleet_ships_total(const fleet_ships_t *fs);
int fleet_ships_get(const fleet_ships_t *fs, const char *type);
void fleet_ships_set(fleet_ships_t *fs, const char *type, int count);
void fleet_ships_add(fleet_ships_t *fs, const char *type, int count);
void fleet_ships_sub(fleet_ships_t *fs, const char *type, int count);
int fleet_ships_has_enough(const fleet_ships_t *fs, const fleet_ships_t *other);
void fleet_ships_sum(fleet_ships_t *fs, const fleet_ships_t *other);
int64_t fleet_ships_cargo_capacity(const fleet_ships_t *fs);
void fleet_ships_each(const fleet_ships_t *fs, void (*fn)(const char *type, int count, void *ctx), void *ctx);

typedef struct {
    int rocket_launcher;
    int light_laser;
    int heavy_laser;
    int ion_cannon;
    int mk2_cannon;
    int plasma_cannon;
    int proton_cannon;
    int small_shield_dome;
    int large_shield_dome;
} defense_ships_t;

typedef struct {
    entity_id_t id;
    entity_id_t owner_id;
    entity_id_t origin_id;
    int target_galaxy;
    int target_system;
    int target_position;
    int target_is_moon;
    int mission_type;
    time_t departure_time;
    time_t arrival_time;
    time_t return_time;
    fleet_ships_t ships;
    int recalled;
    double metal;
    double crystal;
    double gas;
} fleet_mission_t;

typedef struct {
    entity_id_t id;
    entity_id_t attacker_id;
    entity_id_t target_id;
    entity_id_t coordinate_id;
    time_t timestamp;
    char outcome[32];
    int metal;
    int crystal;
    int gas;
    int debris_metal;
    int debris_crystal;
    double moon_chance;
    int moon_created;
} spy_report_t;

typedef struct {
    entity_id_t user_id;
    int64_t total_metal;
    int64_t total_crystal;
    int64_t total_gas;
} power_rank_entry_t;

typedef struct {
    entity_id_t id;
    entity_id_t user_id;
    char commander_type[32];
    int tier;
    time_t expires_at;
} commander_t;

typedef struct {
    entity_id_t id;
    char name[64];
    entity_id_t owner_id;
    time_t created_at;
    char bulletin[1024];
} alliance_t;

typedef struct {
    entity_id_t id;
    entity_id_t alliance_id;
    entity_id_t user_id;
    char username[64];
    char role[32];
} alliance_member_t;

typedef struct {
    entity_id_t id;
    entity_id_t sender_id;
    char sender_name[64];
    entity_id_t recipient_id;
    entity_id_t alliance_id;
    char subject[256];
    char body[4096];
    time_t sent_at;
    int is_read;
} message_t;

typedef struct {
    entity_id_t id;
    entity_id_t alliance_id;
    entity_id_t user_id;
    char username[64];
    char status[32];
    time_t created_at;
} join_request_t;

typedef struct {
    entity_id_t id;
    entity_id_t user_id;
    char galactonite_type[32];
    int stars;
    int level;
    int equipped;
} galactonite_t;

typedef struct {
    entity_id_t user_id;
    int64_t amount;
} crystal_energy_t;

typedef struct {
    entity_id_t id;
    entity_id_t user_id;
    entity_id_t planet_id;
    entity_id_t galactonite_id;
    int slot_index;
} equipped_slot_t;

typedef struct {
    entity_id_t id;
    entity_id_t attacker_id;
    entity_id_t defender_id;
    entity_id_t coordinate_id;
    time_t timestamp;
    char outcome[32];
    char rounds_json[16384];
    int64_t debris_metal;
    int64_t debris_crystal;
    double moon_chance;
    int moon_created;
    double loot_metal;
    double loot_crystal;
    double loot_gas;
} battle_report_t;

typedef struct {
    entity_id_t coordinate_id;
    int64_t metal;
    int64_t crystal;
} debris_field_t;

typedef struct {
    int id;
    char category[32];
    char quest_type[64];
    int target_value;
    int sort_order;
    char prerequisites[512];
    char rewards[1024];
    char title[256];
    char description[1024];
} quest_definition_t;

typedef struct {
    entity_id_t user_id;
    int quest_id;
    int progress;
    int completed;
    int claimed;
    time_t started_at;
    time_t completed_at;
} user_quest_t;

typedef struct {
    entity_id_t id;
    entity_id_t alliance_id;
    entity_id_t initiator_id;
    time_t expires_at;
    char status[32];
    int yea_count;
    int nay_count;
    time_t created_at;
} president_vote_t;

typedef struct {
    entity_id_t id;
    entity_id_t user_id;
    entity_id_t fortress_id;
    int stage;
    time_t last_attack_at;
    int attacks_today;
} fortress_status_t;

typedef struct {
    char username[MAX_USERNAME_LEN];
    char password_hash[MAX_HASH_LEN];
    entity_id_t id;
    entity_id_t server_id;
    int64_t power_rank_score;
    double dark_matter;
    double credits;
    entity_id_t home_planet_id;
    time_t created_at;
    time_t daily_gift_claimed_at;
} user_t;

typedef struct {
    int64_t metal;
    int64_t crystal;
    int64_t gas;
} resource_cost_t;

typedef struct {
    const char *name;
    int64_t metal;
    int64_t crystal;
    int64_t gas;
    int armor;
    int shield;
    int attack;
    int speed;
    int cargo;
    int fuel;
} ship_stats_t;

#endif
