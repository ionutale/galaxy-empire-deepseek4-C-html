#ifndef WORMHOLE_REPO_H
#define WORMHOLE_REPO_H
#include "db.h"
#include "domain/types.h"
#include <time.h>
typedef struct { db_t *db; } wormhole_repo_t;
wormhole_repo_t *wormhole_repo_init(db_t *db);
int wormhole_repo_prepare(wormhole_repo_t *r);
int wormhole_repo_get_cooldown(wormhole_repo_t *r, entity_id_t user_id, entity_id_t moon_a, entity_id_t moon_b, time_t *cooldown_until);
int wormhole_repo_set_cooldown(wormhole_repo_t *r, entity_id_t user_id, entity_id_t moon_a, entity_id_t moon_b, time_t cooldown_until);
#endif
