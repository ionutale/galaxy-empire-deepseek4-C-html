#include "user_repo.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

user_repo_t *user_repo_init(db_t *db) {
    user_repo_t *r = calloc(1, sizeof(user_repo_t));
    if (r) r->db = db;
    return r;
}

int user_repo_prepare(user_repo_t *r) {
    if (!db_prepare(r->db, "user_create",
        "INSERT INTO users (username, password_hash, server_id) VALUES ($1, $2, 1) RETURNING id"))
        return 0;
    if (!db_prepare(r->db, "user_find_username",
        "SELECT id, username, password_hash, server_id, power_rank_score, dark_matter, credits, home_planet_id "
        "FROM users WHERE username = $1"))
        return 0;
    if (!db_prepare(r->db, "user_find_id",
        "SELECT id, username, password_hash, server_id, power_rank_score, dark_matter, credits, home_planet_id "
        "FROM users WHERE id = $1"))
        return 0;
    if (!db_prepare(r->db, "user_exists",
        "SELECT 1 FROM users WHERE username = $1"))
        return 0;
    return 1;
}

int user_repo_create(user_repo_t *r, const char *username, const char *hash, entity_id_t *out_id) {
    const char *params[2] = {username, hash};
    PGresult *res = PQexecPrepared(r->db->conn, "user_create", 2, params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        return 0;
    }
    *out_id = atoll(PQgetvalue(res, 0, 0));
    PQclear(res);
    return 1;
}

static void user_from_result(PGresult *res, user_t *u) {
    u->id = atoll(PQgetvalue(res, 0, 0));
    strncpy(u->username, PQgetvalue(res, 0, 1), sizeof(u->username) - 1);
    strncpy(u->password_hash, PQgetvalue(res, 0, 2), sizeof(u->password_hash) - 1);
    u->server_id = atoll(PQgetvalue(res, 0, 3));
    u->power_rank_score = atoll(PQgetvalue(res, 0, 4));
    u->dark_matter = atof(PQgetvalue(res, 0, 5));
    u->credits = atof(PQgetvalue(res, 0, 6));
    u->home_planet_id = atoll(PQgetvalue(res, 0, 7));
}

user_t *user_repo_find_by_username(user_repo_t *r, const char *username) {
    const char *params[1] = {username};
    PGresult *res = PQexecPrepared(r->db->conn, "user_find_username", 1, params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        PQclear(res);
        return NULL;
    }
    user_t *u = calloc(1, sizeof(user_t));
    user_from_result(res, u);
    PQclear(res);
    return u;
}

user_t *user_repo_find_by_id(user_repo_t *r, entity_id_t id) {
    char id_str[32];
    snprintf(id_str, sizeof(id_str), "%lld", (long long)id);
    const char *params[1] = {id_str};
    PGresult *res = PQexecPrepared(r->db->conn, "user_find_id", 1, params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        PQclear(res);
        return NULL;
    }
    user_t *u = calloc(1, sizeof(user_t));
    user_from_result(res, u);
    PQclear(res);
    return u;
}

int user_repo_exists(user_repo_t *r, const char *username) {
    const char *params[1] = {username};
    PGresult *res = PQexecPrepared(r->db->conn, "user_exists", 1, params, NULL, NULL, 0);
    int exists = (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0);
    PQclear(res);
    return exists;
}
