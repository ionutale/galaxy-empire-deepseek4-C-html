#include "planet_repo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

planet_repo_t *planet_repo_init(db_t *db) {
    planet_repo_t *r = calloc(1, sizeof(planet_repo_t));
    if (r) r->db = db;
    return r;
}

int planet_repo_prepare(planet_repo_t *r) {
    if (!db_prepare(r->db, "planet_create",
        "INSERT INTO planets (user_id, coordinate_id, metal, crystal, gas, energy) "
        "VALUES ($1, $2, 500, 500, 500, 100) RETURNING id"))
        return 0;
    if (!db_prepare(r->db, "planet_find_id",
        "SELECT id, user_id, coordinate_id, metal, crystal, gas, energy, last_updated "
        "FROM planets WHERE id = $1"))
        return 0;
    if (!db_prepare(r->db, "planet_find_user",
        "SELECT id, user_id, coordinate_id, metal, crystal, gas, energy, last_updated "
        "FROM planets WHERE user_id = $1"))
        return 0;
    return 1;
}

int planet_repo_create_home(planet_repo_t *r, entity_id_t user_id, entity_id_t coord_id, entity_id_t *out_id) {
    char uid_str[32], cid_str[32];
    snprintf(uid_str, sizeof(uid_str), "%lld", (long long)user_id);
    snprintf(cid_str, sizeof(cid_str), "%lld", (long long)coord_id);

    const char *params[2] = {uid_str, cid_str};
    PGresult *res = PQexecPrepared(r->db->conn, "planet_create", 2, params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        return 0;
    }
    *out_id = atoll(PQgetvalue(res, 0, 0));
    PQclear(res);
    return 1;
}

static planet_t *planet_from_result(PGresult *res, int row) {
    planet_t *p = calloc(1, sizeof(planet_t));
    if (!p) return NULL;
    p->id = atoll(PQgetvalue(res, row, 0));
    p->user_id = atoll(PQgetvalue(res, row, 1));
    p->coordinate_id = atoll(PQgetvalue(res, row, 2));
    p->metal = atof(PQgetvalue(res, row, 3));
    p->crystal = atof(PQgetvalue(res, row, 4));
    p->gas = atof(PQgetvalue(res, row, 5));
    p->energy = atof(PQgetvalue(res, row, 6));
    p->last_updated = atol(PQgetvalue(res, row, 7));
    return p;
}

planet_t *planet_repo_find_by_id(planet_repo_t *r, entity_id_t id) {
    char id_str[32];
    snprintf(id_str, sizeof(id_str), "%lld", (long long)id);

    const char *params[1] = {id_str};
    PGresult *res = PQexecPrepared(r->db->conn, "planet_find_id", 1, params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        PQclear(res);
        return NULL;
    }
    planet_t *p = planet_from_result(res, 0);
    PQclear(res);
    return p;
}

planet_t *planet_repo_find_by_user_id(planet_repo_t *r, entity_id_t user_id,
                                       entity_id_t *ids, int *count, int max) {
    char uid_str[32];
    snprintf(uid_str, sizeof(uid_str), "%lld", (long long)user_id);

    const char *params[1] = {uid_str};
    PGresult *res = PQexecPrepared(r->db->conn, "planet_find_user", 1, params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        return NULL;
    }

    int n = PQntuples(res);
    if (n == 0) {
        PQclear(res);
        return NULL;
    }

    planet_t *planets = calloc(n, sizeof(planet_t));
    for (int i = 0; i < n && i < max; i++) {
        planets[i].id = atoll(PQgetvalue(res, i, 0));
        planets[i].user_id = atoll(PQgetvalue(res, i, 1));
        planets[i].coordinate_id = atoll(PQgetvalue(res, i, 2));
        planets[i].metal = atof(PQgetvalue(res, i, 3));
        planets[i].crystal = atof(PQgetvalue(res, i, 4));
        planets[i].gas = atof(PQgetvalue(res, i, 5));
        planets[i].energy = atof(PQgetvalue(res, i, 6));
        planets[i].last_updated = atol(PQgetvalue(res, i, 7));
        if (ids) ids[i] = planets[i].id;
    }
    *count = n;
    PQclear(res);
    return planets;
}
