#ifndef COMMANDER_REPO_H
#define COMMANDER_REPO_H
#include "db.h"
#include "domain/types.h"
typedef struct { db_t *db; } commander_repo_t;
commander_repo_t *commander_repo_init(db_t *db);
int commander_repo_prepare(commander_repo_t *r);
int commander_repo_find_active(commander_repo_t *r, entity_id_t user_id, commander_t *out, int *count, int max);
int commander_repo_find_active_by_type(commander_repo_t *r, entity_id_t user_id, const char *type, commander_t *out);
int commander_repo_hire(commander_repo_t *r, entity_id_t user_id, const char *type, int tier, time_t expires_at);
#endif
