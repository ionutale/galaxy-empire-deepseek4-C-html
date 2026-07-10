#ifndef COMBAT_H
#define COMBAT_H

#include <stdint.h>

#define MAX_ROUNDS 6
#define MAX_GROUPS 32
#define MAX_ATTACKS_PER_ROUND 1024

typedef struct {
    int weapons_tech;
    int shielding_tech;
    int strength_tech;
} combat_tech_buffs_t;

typedef struct {
    char type[32];
    int count;
    int64_t attack;
    int64_t shield;
    int64_t armor;
    int64_t cargo;
    int64_t metal;
    int64_t crystal;
} combat_ship_group_t;

typedef struct {
    combat_ship_group_t attacker_groups[MAX_GROUPS];
    int num_attacker_groups;
    combat_ship_group_t defender_groups[MAX_GROUPS];
    int num_defender_groups;
    combat_tech_buffs_t attacker_tech;
    combat_tech_buffs_t defender_tech;
    int64_t random_seed;
} combat_input_t;

typedef struct {
    char attacker_type[32];
    char defender_type[32];
    int64_t damage;
    int64_t shield_before;
    int64_t armor_before;
    int64_t shield_after;
    int64_t armor_after;
    int destroyed;
    int critical_hit;
    int rapid_fire;
} combat_attack_log_t;

typedef struct {
    int number;
    combat_attack_log_t attacks[MAX_ATTACKS_PER_ROUND];
    int num_attacks;
} combat_round_log_t;

typedef struct {
    char outcome[32];
    combat_round_log_t rounds[MAX_ROUNDS];
    int num_rounds;
    combat_ship_group_t attacker_remaining[MAX_GROUPS];
    int num_attacker_remaining;
    combat_ship_group_t defender_remaining[MAX_GROUPS];
    int num_defender_remaining;
    int64_t debris_metal;
    int64_t debris_crystal;
    double moon_chance;
    int defenses_repaired;
} combat_output_t;

combat_output_t combat_simulate(combat_input_t input);

#endif
