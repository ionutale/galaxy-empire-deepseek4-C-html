#ifndef JOIN_REQUEST_REPO_H
#define JOIN_REQUEST_REPO_H
#include "db.h"
#include "domain/types.h"
typedef struct { db_t *db; } join_request_repo_t;
join_request_repo_t *jr_repo_init(db_t *db);
int jr_repo_prepare(join_request_repo_t *r);
int jr_repo_create(join_request_repo_t *r, entity_id_t alliance_id, entity_id_t user_id);
int jr_repo_find_by_alliance(join_request_repo_t *r, entity_id_t alliance_id, join_request_t *out, int *count, int max);
int jr_repo_find_pending_by_user(join_request_repo_t *r, entity_id_t user_id, join_request_t *out);
int jr_repo_update_status(join_request_repo_t *r, entity_id_t alliance_id, entity_id_t user_id, const char *status);
#endif
