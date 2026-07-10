#ifndef ALLIANCE_MEMBER_REPO_H
#define ALLIANCE_MEMBER_REPO_H
#include "db.h"
#include "domain/types.h"
typedef struct { db_t *db; } alliance_member_repo_t;
alliance_member_repo_t *am_repo_init(db_t *db);
int am_repo_prepare(alliance_member_repo_t *r);
int am_repo_add(alliance_member_repo_t *r, entity_id_t alliance_id, entity_id_t user_id, const char *role);
int am_repo_remove(alliance_member_repo_t *r, entity_id_t alliance_id, entity_id_t user_id);
int am_repo_find_by_alliance(alliance_member_repo_t *r, entity_id_t alliance_id, alliance_member_t *out, int *count, int max);
int am_repo_find_by_user(alliance_member_repo_t *r, entity_id_t user_id, alliance_member_t *out);
int am_repo_update_role(alliance_member_repo_t *r, entity_id_t alliance_id, entity_id_t user_id, const char *role);
int am_repo_count(alliance_member_repo_t *r, entity_id_t alliance_id);
#endif
