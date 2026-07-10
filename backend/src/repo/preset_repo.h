#ifndef PRESET_REPO_H
#define PRESET_REPO_H
#include "db.h"
#include "domain/types.h"
typedef struct { db_t *db; } preset_repo_t;
preset_repo_t *preset_repo_init(db_t *db);
int preset_repo_prepare(preset_repo_t *r);
int preset_repo_find_by_user(preset_repo_t *r, entity_id_t user_id, fleet_ships_t *ships, char (*names)[64], int *count, int max);
int preset_repo_create(preset_repo_t *r, entity_id_t user_id, const char *name, const fleet_ships_t *ships, entity_id_t *out_id);
int preset_repo_delete(preset_repo_t *r, entity_id_t id, entity_id_t user_id);
#endif
