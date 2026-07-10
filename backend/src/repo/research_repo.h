#ifndef RESEARCH_REPO_H
#define RESEARCH_REPO_H
#include "db.h"
#include "domain/types.h"
typedef struct { db_t *db; } research_repo_t;
research_repo_t *research_repo_init(db_t *db);
int research_repo_prepare(research_repo_t *r);
int research_repo_find_by_user(research_repo_t *r, entity_id_t user_id, research_t *out);
int research_repo_upsert(research_repo_t *r, const research_t *rs);
void research_repo_clear_queue(research_repo_t *r, entity_id_t user_id);
#endif
