#ifndef DEFENSE_INVENTORY_REPO_H
#define DEFENSE_INVENTORY_REPO_H
#include "db.h"
#include "domain/types.h"
typedef struct { db_t *db; } defense_inventory_repo_t;
defense_inventory_repo_t *def_inv_repo_init(db_t *db);
int def_inv_repo_prepare(defense_inventory_repo_t *r);
int def_inv_repo_find(defense_inventory_repo_t *r, entity_id_t planet_id, defense_ships_t *out);
int def_inv_repo_upsert(defense_inventory_repo_t *r, entity_id_t planet_id, const defense_ships_t *d);
#endif
