#include "combat.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#define MAX_SHIELDS 65536
#define MAX_ARMORS 65536

typedef struct {
    char type[32];
    int count;
    int orig_count;
    int64_t max_shield;
    int64_t max_armor;
    int64_t attack;
    int64_t shots;
    int64_t *current_shield;
    int64_t *current_armor;
    int64_t max_shield_val;
    int64_t max_armor_val;
    int64_t cargo;
    int64_t metal;
    int64_t crystal;
    int is_defense;
} group_state_t;

static const char *defense_types[] = {
    "rocket_launcher", "light_laser", "heavy_laser", "ion_cannon",
    "mk2_cannon", "plasma_cannon", "proton_cannon",
    "small_shield_dome", "large_shield_dome", NULL
};

static const char *excluded_from_debris[] = {"annihilator", NULL};

static int is_defense(const char *type) {
    for (int i = 0; defense_types[i]; i++)
        if (strcmp(defense_types[i], type) == 0) return 1;
    return 0;
}

static int is_excluded_debris(const char *type) {
    for (int i = 0; excluded_from_debris[i]; i++)
        if (strcmp(excluded_from_debris[i], type) == 0) return 1;
    return 0;
}

static int is_combat_ship(const char *type) {
    return strcmp(type, "probe") != 0 &&
           strcmp(type, "solar_satellite") != 0 &&
           strcmp(type, "recycler") != 0 &&
           strcmp(type, "colony_ship") != 0 &&
           strcmp(type, "small_cargo") != 0 &&
           strcmp(type, "large_cargo") != 0;
}

static int check_rapid_fire(const char *attacker, const char *defender) {
    typedef struct { const char *attacker; const char *defender; int rf; } rf_entry_t;
    static rf_entry_t table[] = {
        {"cruiser", "light_fighter", 6},
        {"cruiser", "heavy_fighter", 4},
        {"dreadnought", "battleship", 7},
        {"dreadnought", "cruiser", 4},
        {"dreadnought", "heavy_fighter", 4},
        {"destroyer", "dreadnought", 2},
        {"destroyer", "probe", 10},
        {"bomber", "rocket_launcher", 20},
        {"bomber", "light_laser", 20},
        {"bomber", "heavy_laser", 10},
        {"bomber", "ion_cannon", 10},
    };
    int rf = 0;
    for (int i = 0; i < (int)(sizeof(table)/sizeof(table[0])); i++) {
        if (strcmp(table[i].attacker, attacker) == 0 && strcmp(table[i].defender, defender) == 0) {
            rf = table[i].rf;
            break;
        }
    }
    if (rf == 0 && is_combat_ship(attacker) &&
        (strcmp(defender, "probe") == 0 || strcmp(defender, "solar_satellite") == 0)) {
        rf = 5;
    }
    if (rf == 0) return 0;
    return ((double)rand() / RAND_MAX) < (1.0 - 1.0 / rf);
}

static group_state_t clone_group(combat_ship_group_t g) {
    group_state_t s;
    memset(&s, 0, sizeof(s));
    strncpy(s.type, g.type, sizeof(s.type) - 1);
    s.count = g.count;
    s.orig_count = g.count;
    s.max_shield = g.shield;
    s.max_armor = g.armor;
    s.attack = g.attack;
    s.shots = g.count;
    s.max_shield_val = g.shield;
    s.max_armor_val = g.armor;
    s.cargo = g.cargo;
    s.metal = g.metal;
    s.crystal = g.crystal;
    s.is_defense = is_defense(g.type);
    s.current_shield = malloc(g.count * sizeof(int64_t));
    s.current_armor = malloc(g.count * sizeof(int64_t));
    for (int i = 0; i < g.count; i++) {
        s.current_shield[i] = g.shield;
        s.current_armor[i] = g.armor;
    }
    return s;
}

static void free_group(group_state_t *s) {
    free(s->current_shield);
    free(s->current_armor);
}

static void apply_tech_buffs(combat_ship_group_t *groups, int num_groups, combat_tech_buffs_t tech) {
    double attack_mult = 1.0 + tech.weapons_tech * 0.1;
    double shield_mult = 1.0 + tech.shielding_tech * 0.1;
    double armor_mult = 1.0 + tech.strength_tech * 0.1;
    for (int i = 0; i < num_groups; i++) {
        groups[i].attack = (int64_t)round(groups[i].attack * attack_mult);
        groups[i].shield = (int64_t)round(groups[i].shield * shield_mult);
        groups[i].armor = (int64_t)round(groups[i].armor * armor_mult);
    }
}

static int total_count(group_state_t *groups, int num_groups) {
    int n = 0;
    for (int i = 0; i < num_groups; i++) n += groups[i].count;
    return n;
}

static int pick_target(group_state_t *groups, int num_groups) {
    int t = total_count(groups, num_groups);
    if (t == 0) return -1;
    int r = rand() % t;
    for (int i = 0; i < num_groups; i++) {
        r -= groups[i].count;
        if (r < 0) return i;
    }
    return 0;
}

static int64_t calc_debris_metal(group_state_t *attackers, int na, group_state_t *defenders, int nd) {
    int64_t total = 0;
    for (int i = 0; i < na; i++) {
        if (is_excluded_debris(attackers[i].type) || attackers[i].is_defense) continue;
        int lost = attackers[i].orig_count - attackers[i].count;
        if (lost > 0) total += (int64_t)lost * attackers[i].metal;
    }
    for (int i = 0; i < nd; i++) {
        if (is_excluded_debris(defenders[i].type) || defenders[i].is_defense) continue;
        int lost = defenders[i].orig_count - defenders[i].count;
        if (lost > 0) total += (int64_t)lost * defenders[i].metal;
    }
    return (int64_t)round(total * 0.3);
}

static int64_t calc_debris_crystal(group_state_t *attackers, int na, group_state_t *defenders, int nd) {
    int64_t total = 0;
    for (int i = 0; i < na; i++) {
        if (is_excluded_debris(attackers[i].type) || attackers[i].is_defense) continue;
        int lost = attackers[i].orig_count - attackers[i].count;
        if (lost > 0) total += (int64_t)lost * attackers[i].crystal;
    }
    for (int i = 0; i < nd; i++) {
        if (is_excluded_debris(defenders[i].type) || defenders[i].is_defense) continue;
        int lost = defenders[i].orig_count - defenders[i].count;
        if (lost > 0) total += (int64_t)lost * defenders[i].crystal;
    }
    return (int64_t)round(total * 0.3);
}

static int calc_defense_repair(group_state_t *groups, int ng) {
    int total = 0;
    for (int i = 0; i < ng; i++) {
        if (!groups[i].is_defense) continue;
        int lost = groups[i].orig_count - groups[i].count;
        if (lost > 0) total += lost;
    }
    if (total == 0) return 0;
    return (int)floor(total * 0.7);
}

static void process_shot(group_state_t *shooter, group_state_t *targets, int num_targets,
                          combat_attack_log_t *attacks, int *num_attacks) {
    if (shooter->count <= 0 || total_count(targets, num_targets) == 0) return;

    int ti = pick_target(targets, num_targets);
    if (ti < 0) return;

    group_state_t *target = &targets[ti];
    int ui = rand() % target->count;
    int64_t damage = shooter->attack;

    int64_t sb = target->current_shield[ui];
    int64_t ab = target->current_armor[ui];

    if (damage > 0 && damage < target->max_shield_val / 100) {
        combat_attack_log_t *a = &attacks[(*num_attacks)++];
        strncpy(a->attacker_type, shooter->type, sizeof(a->attacker_type) - 1);
        strncpy(a->defender_type, target->type, sizeof(a->defender_type) - 1);
        a->damage = damage;
        a->shield_before = sb;
        a->armor_before = ab;
        a->shield_after = sb;
        a->armor_after = ab;
        a->destroyed = 0;
        a->critical_hit = 0;
        a->rapid_fire = 0;
        return;
    }

    int64_t overflow = damage - target->current_shield[ui];
    if (overflow <= 0) {
        target->current_shield[ui] -= damage;
        combat_attack_log_t *a = &attacks[(*num_attacks)++];
        strncpy(a->attacker_type, shooter->type, sizeof(a->attacker_type) - 1);
        strncpy(a->defender_type, target->type, sizeof(a->defender_type) - 1);
        a->damage = damage;
        a->shield_before = sb;
        a->armor_before = ab;
        a->shield_after = target->current_shield[ui];
        a->armor_after = ab;
        a->destroyed = 0;
        a->critical_hit = 0;
        a->rapid_fire = 0;
        return;
    }

    target->current_shield[ui] = 0;
    target->current_armor[ui] -= overflow;

    int destroyed = 0;
    int crit = 0;

    if (target->current_armor[ui] <= 0) {
        destroyed = 1;
    } else {
        double pct = (double)target->current_armor[ui] / (double)target->max_armor_val;
        if (pct < 0.3 && ((double)rand() / RAND_MAX) < (1.0 - pct)) {
            destroyed = 1;
            crit = 1;
        }
    }

    if (destroyed) {
        int last = target->count - 1;
        if (ui != last) {
            target->current_shield[ui] = target->current_shield[last];
            target->current_armor[ui] = target->current_armor[last];
        }
        target->count--;
    }

    int64_t sa = (destroyed && ui >= target->count) ? 0 : target->current_shield[ui];
    int64_t aa = (destroyed && ui >= target->count) ? 0 : target->current_armor[ui];

    combat_attack_log_t *a = &attacks[(*num_attacks)++];
    strncpy(a->attacker_type, shooter->type, sizeof(a->attacker_type) - 1);
    strncpy(a->defender_type, target->type, sizeof(a->defender_type) - 1);
    a->damage = damage;
    a->shield_before = sb;
    a->armor_before = ab;
    a->shield_after = sa;
    a->armor_after = aa;
    a->destroyed = destroyed;
    a->critical_hit = crit;
    a->rapid_fire = 0;

    if (destroyed && check_rapid_fire(shooter->type, target->type)) {
        shooter->shots++;
    }
}

static void to_output_groups(group_state_t *states, int num_states,
                               combat_ship_group_t *out, int *num_out) {
    *num_out = 0;
    for (int i = 0; i < num_states; i++) {
        if (states[i].count > 0) {
            combat_ship_group_t *g = &out[(*num_out)++];
            strncpy(g->type, states[i].type, sizeof(g->type) - 1);
            g->count = states[i].count;
            g->attack = states[i].attack;
            g->shield = states[i].max_shield_val;
            g->armor = states[i].max_armor_val;
            g->cargo = states[i].cargo;
            g->metal = states[i].metal;
            g->crystal = states[i].crystal;
        }
    }
}

combat_output_t combat_simulate(combat_input_t input) {
    combat_output_t output;
    memset(&output, 0, sizeof(output));

    srand((unsigned int)input.random_seed);

    apply_tech_buffs(input.attacker_groups, input.num_attacker_groups, input.attacker_tech);
    apply_tech_buffs(input.defender_groups, input.num_defender_groups, input.defender_tech);

    group_state_t attackers[MAX_GROUPS];
    group_state_t defenders[MAX_GROUPS];
    int na = 0, nd = 0;

    for (int i = 0; i < input.num_attacker_groups; i++)
        attackers[na++] = clone_group(input.attacker_groups[i]);
    for (int i = 0; i < input.num_defender_groups; i++)
        defenders[nd++] = clone_group(input.defender_groups[i]);

    for (int round_num = 0; round_num < MAX_ROUNDS; round_num++) {
        if (total_count(attackers, na) == 0 || total_count(defenders, nd) == 0) break;

        combat_round_log_t *r = &output.rounds[output.num_rounds++];
        r->number = round_num + 1;
        r->num_attacks = 0;

        for (int ai = 0; ai < na; ai++) {
            if (attackers[ai].count <= 0) continue;
            for (int64_t s = 0; s < attackers[ai].shots; s++) {
                if (attackers[ai].count <= 0 || total_count(defenders, nd) == 0) break;
                process_shot(&attackers[ai], defenders, nd, r->attacks, &r->num_attacks);
            }
        }

        for (int di = 0; di < nd; di++) {
            if (defenders[di].count <= 0) continue;
            for (int64_t s = 0; s < defenders[di].shots; s++) {
                if (defenders[di].count <= 0 || total_count(attackers, na) == 0) break;
                process_shot(&defenders[di], attackers, na, r->attacks, &r->num_attacks);
            }
        }

        for (int i = 0; i < na; i++) {
            for (int j = 0; j < attackers[i].count; j++)
                attackers[i].current_shield[j] = attackers[i].max_shield_val;
            attackers[i].shots = attackers[i].count;
        }
        for (int i = 0; i < nd; i++) {
            for (int j = 0; j < defenders[i].count; j++)
                defenders[i].current_shield[j] = defenders[i].max_shield_val;
            defenders[i].shots = defenders[i].count;
        }
    }

    int ac = total_count(attackers, na);
    int dc = total_count(defenders, nd);

    if (ac > 0 && dc == 0)
        strncpy(output.outcome, "attacker_victory", sizeof(output.outcome) - 1);
    else if (dc > 0 && ac == 0)
        strncpy(output.outcome, "defender_victory", sizeof(output.outcome) - 1);
    else
        strncpy(output.outcome, "draw", sizeof(output.outcome) - 1);

    to_output_groups(attackers, na, output.attacker_remaining, &output.num_attacker_remaining);
    to_output_groups(defenders, nd, output.defender_remaining, &output.num_defender_remaining);

    output.debris_metal = calc_debris_metal(attackers, na, defenders, nd);
    output.debris_crystal = calc_debris_crystal(attackers, na, defenders, nd);
    output.moon_chance = fmin(0.20, (double)(output.debris_metal + output.debris_crystal) / 400000.0);
    output.defenses_repaired = calc_defense_repair(defenders, nd);

    for (int i = 0; i < na; i++) free_group(&attackers[i]);
    for (int i = 0; i < nd; i++) free_group(&defenders[i]);

    return output;
}
