#ifndef USER_REPO_H
#define USER_REPO_H

#include "db.h"
#include "domain/types.h"

typedef struct {
    db_t *db;
} user_repo_t;

user_repo_t *user_repo_init(db_t *db);
int user_repo_prepare(user_repo_t *r);
int user_repo_create(user_repo_t *r, const char *username, const char *hash, entity_id_t *out_id);
user_t *user_repo_find_by_username(user_repo_t *r, const char *username);
user_t *user_repo_find_by_id(user_repo_t *r, entity_id_t id);
int user_repo_exists(user_repo_t *r, const char *username);

#endif
