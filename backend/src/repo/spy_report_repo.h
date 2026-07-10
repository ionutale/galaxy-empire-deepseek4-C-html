#ifndef SPY_REPORT_REPO_H
#define SPY_REPORT_REPO_H
#include "db.h"
#include "domain/types.h"
typedef struct { db_t *db; } spy_report_repo_t;
spy_report_repo_t *sr_repo_init(db_t *db);
int sr_repo_prepare(spy_report_repo_t *r);
int sr_repo_create(spy_report_repo_t *r, const spy_report_t *report);
int sr_repo_find_by_attacker(spy_report_repo_t *r, entity_id_t attacker_id, spy_report_t *out, int *count, int max);
#endif
