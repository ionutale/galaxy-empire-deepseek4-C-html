#ifndef ALLIANCE_REPO_H
#define ALLIANCE_REPO_H
#include "db.h"
#include "domain/types.h"
typedef struct { db_t *db; } alliance_repo_t;
alliance_repo_t *alliance_repo_init(db_t *db);
int alliance_repo_prepare(alliance_repo_t *r);
int alliance_repo_create(alliance_repo_t *r, const char *name, entity_id_t owner_id, entity_id_t *out_id);
int alliance_repo_find_by_id(alliance_repo_t *r, entity_id_t id, alliance_t *out);
int alliance_repo_find_by_user(alliance_repo_t *r, entity_id_t user_id, alliance_t *out);
int alliance_repo_update_bulletin(alliance_repo_t *r, entity_id_t id, const char *bulletin);
int alliance_repo_delete(alliance_repo_t *r, entity_id_t id);
#endif
