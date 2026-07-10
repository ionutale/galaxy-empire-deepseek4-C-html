#ifndef BATTLE_REPORT_REPO_H
#define BATTLE_REPORT_REPO_H
#include "db.h"
#include "domain/types.h"
typedef struct { db_t *db; } battle_report_repo_t;
battle_report_repo_t *br_repo_init(db_t *db);
int br_repo_prepare(battle_report_repo_t *r);
int br_repo_create(battle_report_repo_t *r, const battle_report_t *report);
int br_repo_find_by_player(battle_report_repo_t *r, entity_id_t player_id, battle_report_t *out, int *count, int max);
int br_repo_find_by_id(battle_report_repo_t *r, entity_id_t id, battle_report_t *out);
#endif
