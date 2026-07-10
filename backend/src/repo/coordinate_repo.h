#ifndef COORDINATE_REPO_H
#define COORDINATE_REPO_H

#include "db.h"
#include "domain/types.h"

typedef struct {
    db_t *db;
} coordinate_repo_t;

coordinate_repo_t *coordinate_repo_init(db_t *db);
int coordinate_repo_prepare(coordinate_repo_t *r);
int coordinate_repo_find_next_free(coordinate_repo_t *r, entity_id_t *out_galaxy, entity_id_t *out_system, entity_id_t *out_position);
int coordinate_repo_create(coordinate_repo_t *r, entity_id_t galaxy, entity_id_t system, entity_id_t position, entity_id_t *out_id);
coordinate_t *coordinate_repo_find_by_id(coordinate_repo_t *r, entity_id_t id);

#endif
