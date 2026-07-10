#ifndef PLANET_REPO_H
#define PLANET_REPO_H

#include "db.h"
#include "domain/types.h"

typedef struct {
    db_t *db;
} planet_repo_t;

planet_repo_t *planet_repo_init(db_t *db);
int planet_repo_prepare(planet_repo_t *r);
int planet_repo_create_home(planet_repo_t *r, entity_id_t user_id, entity_id_t coord_id, entity_id_t *out_id);
planet_t *planet_repo_find_by_id(planet_repo_t *r, entity_id_t id);
planet_t *planet_repo_find_by_user_id(planet_repo_t *r, entity_id_t user_id, entity_id_t *ids, int *count, int max);

#endif
