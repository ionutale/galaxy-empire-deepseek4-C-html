#ifndef FORTRESS_REPO_H
#define FORTRESS_REPO_H
#include "db.h"
#include "domain/types.h"
typedef struct { db_t *db; } fortress_repo_t;
fortress_repo_t *fortress_repo_init(db_t *db);
int fortress_repo_prepare(fortress_repo_t *r);
int fortress_repo_get_attack_count(fortress_repo_t *r, entity_id_t user_id, const char *date);
int fortress_repo_increment_attacks(fortress_repo_t *r, entity_id_t user_id, const char *date);
int fortress_repo_get_control(fortress_repo_t *r, entity_id_t *alliance_id);
int fortress_repo_set_control(fortress_repo_t *r, entity_id_t alliance_id);
int fortress_repo_clear_control(fortress_repo_t *r);
int fortress_repo_init_control(fortress_repo_t *r);
#endif
