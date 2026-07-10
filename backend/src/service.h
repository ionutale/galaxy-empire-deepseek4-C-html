#ifndef SERVICE_H
#define SERVICE_H

#include "db.h"
#include "domain/types.h"
#include "repo/user_repo.h"
#include "repo/planet_repo.h"
#include "repo/coordinate_repo.h"
#include "repo/research_repo.h"
#include "repo/fleet_repo.h"
#include "repo/fleet_inventory_repo.h"
#include "repo/defense_inventory_repo.h"
#include "repo/battle_report_repo.h"
#include "repo/spy_report_repo.h"
#include "repo/commander_repo.h"
#include "repo/fortress_repo.h"
#include "repo/wormhole_repo.h"
#include "repo/galactonite_repo.h"
#include "repo/alliance_repo.h"
#include "repo/alliance_member_repo.h"
#include "repo/join_request_repo.h"
#include "repo/president_vote_repo.h"
#include "repo/queue_repo.h"
#include "repo/preset_repo.h"

typedef struct {
    user_repo_t *users;
    planet_repo_t *planets;
    coordinate_repo_t *coordinates;
    research_repo_t *research;
    fleet_repo_t *fleets;
    fleet_inventory_repo_t *fleet_inv;
    defense_inventory_repo_t *def_inv;
    battle_report_repo_t *battle_reports;
    spy_report_repo_t *spy_reports;
    commander_repo_t *commanders;
    fortress_repo_t *fortress;
    wormhole_repo_t *wormhole;
    galactonite_repo_t *galactonites;
    alliance_repo_t *alliances;
    alliance_member_repo_t *alliance_members;
    join_request_repo_t *join_requests;
    president_vote_repo_t *president_votes;
    queue_repo_t *queue;
    preset_repo_t *presets;
} service_repos_t;

typedef struct {
    service_repos_t repos;
    const char *jwt_secret;
} service_t;

service_t *service_init(db_t *db, const char *jwt_secret);
int repos_prepare_all(db_t *db);

// Auth
int service_register(service_t *s, const char *username, const char *password, user_t *out_user, char **out_token, entity_id_t *out_planet_id);
int service_login(service_t *s, const char *username, const char *password, user_t *out_user, char **out_token);

// Planet
int service_get_planet_view(service_t *s, entity_id_t planet_id, planet_t *out_planet);

// Resources
int service_tick_resources(service_t *s);
int service_recalculate_power_ranks(service_t *s);

// Fleet
int service_send_fleet(service_t *s, entity_id_t user_id, entity_id_t origin_id, int galaxy, int system, int pos, int mission_type, const fleet_ships_t *ships, double cargo_metal, double cargo_crystal, double cargo_gas);
int service_process_arrivals(service_t *s);

#endif
