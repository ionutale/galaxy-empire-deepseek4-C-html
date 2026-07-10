#ifndef GALACTONITE_REPO_H
#define GALACTONITE_REPO_H
#include "db.h"
#include "domain/types.h"
typedef struct { db_t *db; } galactonite_repo_t;
galactonite_repo_t *ga_repo_init(db_t *db);
int ga_repo_prepare(galactonite_repo_t *r);
int ga_repo_find_by_owner(galactonite_repo_t *r, entity_id_t user_id, galactonite_t *out, int *count, int max);
int ga_repo_find_by_id(galactonite_repo_t *r, entity_id_t id, galactonite_t *out);
int ga_repo_create(galactonite_repo_t *r, entity_id_t user_id, const char *type, int stars, int level, entity_id_t *out_id);
int ga_repo_update_level(galactonite_repo_t *r, entity_id_t id, int level);
int ga_repo_update_equipped(galactonite_repo_t *r, entity_id_t id, int equipped);
int ga_repo_find_crystal_energy(galactonite_repo_t *r, entity_id_t user_id, int64_t *amount);
int ga_repo_add_crystal_energy(galactonite_repo_t *r, entity_id_t user_id, int64_t amount);
int ga_repo_deduct_crystal_energy(galactonite_repo_t *r, entity_id_t user_id, int64_t amount);
#endif
