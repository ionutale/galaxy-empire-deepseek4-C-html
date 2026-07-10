#ifndef PRESIDENT_VOTE_REPO_H
#define PRESIDENT_VOTE_REPO_H
#include "db.h"
#include "domain/types.h"
typedef struct { db_t *db; } president_vote_repo_t;
president_vote_repo_t *pv_repo_init(db_t *db);
int pv_repo_prepare(president_vote_repo_t *r);
int pv_repo_create(president_vote_repo_t *r, entity_id_t alliance_id, entity_id_t initiator_id, time_t expires_at, entity_id_t *out_id);
int pv_repo_find_active(president_vote_repo_t *r, entity_id_t alliance_id, president_vote_t *out);
int pv_repo_update_status(president_vote_repo_t *r, entity_id_t id, const char *status);
int pv_repo_add_vote(president_vote_repo_t *r, entity_id_t vote_id, entity_id_t user_id, const char *vote);
int pv_repo_has_voted(president_vote_repo_t *r, entity_id_t vote_id, entity_id_t user_id);
#endif
