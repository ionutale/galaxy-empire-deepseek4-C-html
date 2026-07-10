#ifndef QUEUE_REPO_H
#define QUEUE_REPO_H
#include "db.h"
#include "domain/types.h"
typedef struct { db_t *db; } queue_repo_t;
queue_repo_t *queue_repo_init(db_t *db);
int queue_repo_prepare(queue_repo_t *r);
int queue_repo_insert_construction(queue_repo_t *r, entity_id_t planet_id, const char *building, int from_lvl, int to_lvl);
int queue_repo_get_construction(queue_repo_t *r, entity_id_t planet_id, queue_item_t *out, int *count, int max);
int queue_repo_delete_construction(queue_repo_t *r, entity_id_t id);
int queue_repo_pop_first_construction(queue_repo_t *r, entity_id_t planet_id, queue_item_t *out);
int queue_repo_insert_production(queue_repo_t *r, entity_id_t planet_id, const char *unit_type, int quantity);
int queue_repo_get_production(queue_repo_t *r, entity_id_t planet_id, prod_queue_item_t *out, int *count, int max);
int queue_repo_delete_production(queue_repo_t *r, entity_id_t id);
int queue_repo_pop_first_production(queue_repo_t *r, entity_id_t planet_id, prod_queue_item_t *out);
int queue_repo_insert_research(queue_repo_t *r, entity_id_t user_id, const char *tech_name);
int queue_repo_get_research(queue_repo_t *r, entity_id_t user_id, research_queue_item_t *out, int *count, int max);
int queue_repo_delete_research(queue_repo_t *r, entity_id_t id);
int queue_repo_pop_first_research(queue_repo_t *r, entity_id_t user_id, research_queue_item_t *out);
#endif
