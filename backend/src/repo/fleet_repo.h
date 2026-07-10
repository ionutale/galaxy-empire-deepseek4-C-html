#ifndef FLEET_REPO_H
#define FLEET_REPO_H
#include "db.h"
#include "domain/types.h"
typedef struct { db_t *db; } fleet_repo_t;
fleet_repo_t *fleet_repo_init(db_t *db);
int fleet_repo_prepare(fleet_repo_t *r);
int fleet_repo_create(fleet_repo_t *r, const fleet_mission_t *m, entity_id_t *out_id);
int fleet_repo_find_arrived(fleet_repo_t *r, time_t now, fleet_mission_t *out, int *count, int max);
int fleet_repo_find_active(fleet_repo_t *r, entity_id_t owner_id, fleet_mission_t *out, int *count, int max);
int fleet_repo_find_by_id(fleet_repo_t *r, entity_id_t id, fleet_mission_t *out);
int fleet_repo_set_return_time(fleet_repo_t *r, entity_id_t id, time_t t);
int fleet_repo_mark_recalled(fleet_repo_t *r, entity_id_t id);
int fleet_repo_mark_arrival_processed(fleet_repo_t *r, entity_id_t id);
int fleet_repo_delete(fleet_repo_t *r, entity_id_t id);
int fleet_repo_update_ships(fleet_repo_t *r, entity_id_t id, const fleet_ships_t *ships);
int fleet_repo_update(fleet_repo_t *r, const fleet_mission_t *m);
int fleet_repo_find_defense_at(fleet_repo_t *r, int galaxy, int system, int pos, time_t now, fleet_mission_t *out, int *count, int max);
#endif
