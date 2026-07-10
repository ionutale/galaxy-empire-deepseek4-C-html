#ifndef FLEET_INVENTORY_REPO_H
#define FLEET_INVENTORY_REPO_H
#include "db.h"
#include "domain/types.h"
typedef struct { db_t *db; } fleet_inventory_repo_t;
fleet_inventory_repo_t *fleet_inv_repo_init(db_t *db);
int fleet_inv_repo_prepare(fleet_inventory_repo_t *r);
int fleet_inv_repo_find(fleet_inventory_repo_t *r, entity_id_t planet_id, fleet_ships_t *out);
int fleet_inv_repo_upsert(fleet_inventory_repo_t *r, entity_id_t planet_id, const fleet_ships_t *ships);
#endif
