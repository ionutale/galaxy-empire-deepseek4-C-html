#include "service.h"
#include "config.h"
#include "jwt.h"
#include "combat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

static int64_t compute_fuel_ships(const fleet_ships_t *ships) {
    int64_t fuel = 0;
    fuel += (int64_t)ships->light_fighter * 20;
    fuel += (int64_t)ships->heavy_fighter * 75;
    fuel += (int64_t)ships->cruiser * 300;
    fuel += (int64_t)ships->battleship * 500;
    fuel += (int64_t)ships->dreadnought * 1000;
    fuel += (int64_t)ships->destroyer * 2000;
    fuel += (int64_t)ships->small_cargo * 10;
    fuel += (int64_t)ships->large_cargo * 50;
    fuel += (int64_t)ships->recycler * 300;
    fuel += (int64_t)ships->colony_ship * 1000;
    fuel += (int64_t)ships->probe * 1;
    return fuel;
}

static int slowest_ship_speed_f(const fleet_ships_t *ships) {
    static const int speeds[] = {12500,10000,15000,10000,10000,9000,4000,12000,8000,8000,9000,5000,12500,11000,4000,2500,100000000,0};
    static const char *names[] = {"light_fighter","heavy_fighter","cruiser","battleship","dreadnought","destroyer","bomber","interceptor","iron_behemoth","devourer_interceptor","titanenergetic_frigate","annihilator","small_cargo","large_cargo","recycler","colony_ship","probe","solar_satellite",NULL};
    int slowest = 100000000;
    for (int i = 0; names[i]; i++) {
        int cnt = fleet_ships_get(ships, names[i]);
        if (cnt > 0 && speeds[i] < slowest) slowest = speeds[i];
    }
    return slowest < 100 ? 100 : slowest;
}

static void deduct_fleet(fleet_ships_t *fi, const fleet_ships_t *ships) {
    fleet_ships_sub(fi, "light_fighter", ships->light_fighter);
    fleet_ships_sub(fi, "heavy_fighter", ships->heavy_fighter);
    fleet_ships_sub(fi, "cruiser", ships->cruiser);
    fleet_ships_sub(fi, "battleship", ships->battleship);
    fleet_ships_sub(fi, "dreadnought", ships->dreadnought);
    fleet_ships_sub(fi, "destroyer", ships->destroyer);
    fleet_ships_sub(fi, "bomber", ships->bomber);
    fleet_ships_sub(fi, "interceptor", ships->interceptor);
    fleet_ships_sub(fi, "iron_behemoth", ships->iron_behemoth);
    fleet_ships_sub(fi, "devourer_interceptor", ships->devourer_interceptor);
    fleet_ships_sub(fi, "titanenergetic_frigate", ships->titanenergetic_frigate);
    fleet_ships_sub(fi, "annihilator", ships->annihilator);
    fleet_ships_sub(fi, "small_cargo", ships->small_cargo);
    fleet_ships_sub(fi, "large_cargo", ships->large_cargo);
    fleet_ships_sub(fi, "recycler", ships->recycler);
    fleet_ships_sub(fi, "colony_ship", ships->colony_ship);
    fleet_ships_sub(fi, "probe", ships->probe);
    fleet_ships_sub(fi, "solar_satellite", ships->solar_satellite);
}

static void gen_salt(char *out, size_t len) {
    unsigned char bytes[16];
    RAND_bytes(bytes, sizeof(bytes));
    for (int i = 0; i < 16 && i * 2 + 2 <= (int)len; i++) sprintf(out + i * 2, "%02x", bytes[i]);
    out[len - 1] = '\0';
}

static void hash_pw(const char *pw, const char *salt, char *out, size_t out_len) {
    char salted[256];
    snprintf(salted, sizeof(salted), "%s%s", salt, pw);
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char *)salted, strlen(salted), hash);
    char hex[65];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) sprintf(hex + i * 2, "%02x", hash[i]);
    hex[64] = '\0';
    snprintf(out, out_len, "%s:%s", salt, hex);
}

static int verify_pw(const char *pw, const char *stored) {
    char salt[33], expected[65];
    if (sscanf(stored, "%32[^:]:%64s", salt, expected) != 2) return 0;
    char computed[128];
    hash_pw(pw, salt, computed, sizeof(computed));
    const char *c = strchr(computed, ':');
    return c && strcmp(c + 1, expected) == 0;
}

static int64_t flight_dist(int g1, int s1, int p1, int g2, int s2, int p2) {
    int d = abs(g2 - g1) * 20000 + abs(s2 - s1) * 100 + abs(p2 - p1);
    return d < 5 ? 5 : d;
}

int repos_prepare_all(db_t *db) {
    int ok = 1;
    user_repo_t *ur = user_repo_init(db); if (!user_repo_prepare(ur)) { fprintf(stderr,"user_repo_prepare failed\n"); ok=0; } free(ur);
    planet_repo_t *pr = planet_repo_init(db); if (!planet_repo_prepare(pr)) { fprintf(stderr,"planet_repo_prepare failed\n"); ok=0; } free(pr);
    coordinate_repo_t *cr = coordinate_repo_init(db); if (!coordinate_repo_prepare(cr)) { fprintf(stderr,"coord_repo_prepare failed\n"); ok=0; } free(cr);
    research_repo_t *rr = research_repo_init(db); if (!research_repo_prepare(rr)) { fprintf(stderr,"research_repo_prepare failed\n"); ok=0; } free(rr);
    fleet_repo_t *fr = fleet_repo_init(db); if (!fleet_repo_prepare(fr)) { fprintf(stderr,"fleet_repo_prepare failed\n"); ok=0; } free(fr);
    fleet_inventory_repo_t *fi = fleet_inv_repo_init(db); if (!fleet_inv_repo_prepare(fi)) { fprintf(stderr,"fi_repo_prepare failed\n"); ok=0; } free(fi);
    defense_inventory_repo_t *di = def_inv_repo_init(db); if (!def_inv_repo_prepare(di)) { fprintf(stderr,"di_repo_prepare failed\n"); ok=0; } free(di);
    battle_report_repo_t *br = br_repo_init(db); if (!br_repo_prepare(br)) { fprintf(stderr,"br_repo_prepare failed\n"); ok=0; } free(br);
    spy_report_repo_t *sr = sr_repo_init(db); if (!sr_repo_prepare(sr)) { fprintf(stderr,"sr_repo_prepare failed\n"); ok=0; } free(sr);
    commander_repo_t *com = commander_repo_init(db); if (!commander_repo_prepare(com)) { fprintf(stderr,"com_repo_prepare failed\n"); ok=0; } free(com);
    fortress_repo_t *fo = fortress_repo_init(db); if (!fortress_repo_prepare(fo)) { fprintf(stderr,"fo_repo_prepare failed\n"); ok=0; } free(fo);
    wormhole_repo_t *wh = wormhole_repo_init(db); if (!wormhole_repo_prepare(wh)) { fprintf(stderr,"wh_repo_prepare failed\n"); ok=0; } free(wh);
    galactonite_repo_t *ga = ga_repo_init(db); if (!ga_repo_prepare(ga)) { fprintf(stderr,"ga_repo_prepare failed\n"); ok=0; } free(ga);
    alliance_repo_t *al = alliance_repo_init(db); if (!alliance_repo_prepare(al)) { fprintf(stderr,"al_repo_prepare failed\n"); ok=0; } free(al);
    alliance_member_repo_t *am = am_repo_init(db); if (!am_repo_prepare(am)) { fprintf(stderr,"am_repo_prepare failed\n"); ok=0; } free(am);
    join_request_repo_t *jr = jr_repo_init(db); if (!jr_repo_prepare(jr)) { fprintf(stderr,"jr_repo_prepare failed\n"); ok=0; } free(jr);
    president_vote_repo_t *pv = pv_repo_init(db); if (!pv_repo_prepare(pv)) { fprintf(stderr,"pv_repo_prepare failed\n"); ok=0; } free(pv);
    queue_repo_t *q = queue_repo_init(db); if (!queue_repo_prepare(q)) { fprintf(stderr,"queue_repo_prepare failed\n"); ok=0; } free(q);
    preset_repo_t *ps = preset_repo_init(db); if (!preset_repo_prepare(ps)) { fprintf(stderr,"preset_repo_prepare failed\n"); ok=0; } free(ps);
    return ok;
}

service_t *service_init(db_t *db, const char *jwt_secret) {
    service_t *s = calloc(1, sizeof(service_t));
    s->repos.users = user_repo_init(db);
    s->repos.planets = planet_repo_init(db);
    s->repos.coordinates = coordinate_repo_init(db);
    s->repos.research = research_repo_init(db);
    s->repos.fleets = fleet_repo_init(db);
    s->repos.fleet_inv = fleet_inv_repo_init(db);
    s->repos.def_inv = def_inv_repo_init(db);
    s->repos.battle_reports = br_repo_init(db);
    s->repos.spy_reports = sr_repo_init(db);
    s->repos.commanders = commander_repo_init(db);
    s->repos.fortress = fortress_repo_init(db);
    s->repos.wormhole = wormhole_repo_init(db);
    s->repos.galactonites = ga_repo_init(db);
    s->repos.alliances = alliance_repo_init(db);
    s->repos.alliance_members = am_repo_init(db);
    s->repos.join_requests = jr_repo_init(db);
    s->repos.president_votes = pv_repo_init(db);
    s->repos.queue = queue_repo_init(db);
    s->repos.presets = preset_repo_init(db);
    s->jwt_secret = jwt_secret;
    return s;
}

int service_register(service_t *s, const char *username, const char *password, user_t *out_user, char **out_token, entity_id_t *out_planet_id) {
    if (strlen(username) < 3 || strlen(username) > 32) return -1;
    if (strlen(password) < 6) return -1;
    if (user_repo_exists(s->repos.users, username)) return -2;
    char hash[128], salt[33];
    gen_salt(salt, sizeof(salt));
    hash_pw(password, salt, hash, sizeof(hash));
    entity_id_t uid;
    if (!user_repo_create(s->repos.users, username, hash, &uid)) return -3;
    entity_id_t g, sys, pos, cid;
    if (!coordinate_repo_find_next_free(s->repos.coordinates, &g, &sys, &pos)) return -3;
    if (!coordinate_repo_create(s->repos.coordinates, g, sys, pos, &cid)) return -3;
    entity_id_t pid;
    if (!planet_repo_create_home(s->repos.planets, uid, cid, &pid)) return -4;
    char sql[256];
    snprintf(sql, sizeof(sql), "UPDATE users SET home_planet_id=%lld WHERE id=%lld", (long long)pid, (long long)uid);
    db_exec(((db_t *)s->repos.users->db), sql);
    char *token = jwt_create(uid, username, s->jwt_secret);
    if (!token) return -5;
    out_user->id = uid;
    strncpy(out_user->username, username, sizeof(out_user->username)-1);
    out_user->home_planet_id = pid;
    *out_token = token;
    *out_planet_id = pid;
    return 1;
}

int service_login(service_t *s, const char *username, const char *password, user_t *out_user, char **out_token) {
    user_t *user = user_repo_find_by_username(s->repos.users, username);
    if (!user) return 0;
    if (!verify_pw(password, user->password_hash)) { free(user); return 0; }
    char *token = jwt_create(user->id, user->username, s->jwt_secret);
    if (!token) { free(user); return -1; }
    *out_user = *user;
    *out_token = token;
    free(user);
    return 1;
}

int service_send_fleet(service_t *s, entity_id_t uid, entity_id_t oid,
                        int g, int sys, int pos, int mt,
                        const fleet_ships_t *ships,
                        double cm, double cc, double cg) {
    if (fleet_ships_total(ships) == 0) return -1;
    planet_t *p = planet_repo_find_by_id(s->repos.planets, oid);
    if (!p) return -3;
    if (p->user_id != uid) { free(p); return -4; }
    if (cm + cc + cg > fleet_ships_cargo_capacity(ships)) { free(p); return -5; }
    if (p->metal < cm || p->crystal < cc || p->gas < cg) { free(p); return -6; }
    fleet_ships_t fi = fleet_ships_zero();
    fleet_inv_repo_find(s->repos.fleet_inv, oid, &fi);
    if (!fleet_ships_has_enough(&fi, ships)) { free(p); return -7; }
    deduct_fleet(&fi, ships);
    fleet_inv_repo_upsert(s->repos.fleet_inv, oid, &fi);
    coordinate_t *c = coordinate_repo_find_by_id(s->repos.coordinates, p->coordinate_id);
    int64_t d = c ? flight_dist(c->galaxy_num, c->system_num, c->position_num, g, sys, pos) : 1000;
    if (c) free(c);
    int64_t fuel = compute_fuel_ships(ships) * d / 35000;
    if (fuel < 0) fuel = 0;
    double gas_needed = (double)fuel + (mt == 2 ? cg : 0);
    if (p->gas < gas_needed) { free(p); return -8; }
    p->metal -= cm; p->crystal -= cc; p->gas -= gas_needed;
    char sql[256];
    snprintf(sql, sizeof(sql), "UPDATE planets SET metal=%.0f,crystal=%.0f,gas=%.0f WHERE id=%lld",
        p->metal, p->crystal, p->gas, (long long)oid);
    db_exec(((db_t *)s->repos.planets->db), sql);
    int speed = slowest_ship_speed_f(ships);
    int dur = (int)(d * 3500 / speed);
    fleet_mission_t m; memset(&m, 0, sizeof(m));
    m.owner_id = uid; m.origin_id = oid;
    m.target_galaxy = g; m.target_system = sys; m.target_position = pos;
    m.mission_type = mt;
    m.departure_time = time(NULL);
    m.arrival_time = time(NULL) + dur;
    m.return_time = time(NULL) + dur * 2;
    m.ships = *ships; m.metal = cm; m.crystal = cc; m.gas = cg;
    fleet_repo_create(s->repos.fleets, &m, NULL);
    free(p);
    return 1;
}
