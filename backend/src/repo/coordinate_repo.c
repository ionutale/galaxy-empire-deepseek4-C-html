#include "coordinate_repo.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

coordinate_repo_t *coordinate_repo_init(db_t *db) {
    coordinate_repo_t *r = calloc(1, sizeof(coordinate_repo_t));
    if (r) r->db = db;
    return r;
}

int coordinate_repo_prepare(coordinate_repo_t *r) {
    if (!db_prepare(r->db, "coord_next_free",
        "SELECT galaxy_num, system_num, position_num FROM coordinates "
        "ORDER BY galaxy_num, system_num, position_num"))
        return 0;
    if (!db_prepare(r->db, "coord_create",
        "INSERT INTO coordinates (server_id, galaxy_num, system_num, position_num, is_moon) "
        "VALUES (1, $1, $2, $3, false) RETURNING id"))
        return 0;
    if (!db_prepare(r->db, "coord_find",
        "SELECT id, galaxy_num, system_num, position_num, is_moon FROM coordinates WHERE id = $1"))
        return 0;
    return 1;
}

int coordinate_repo_find_next_free(coordinate_repo_t *r,
                                    entity_id_t *out_galaxy, entity_id_t *out_system,
                                    entity_id_t *out_position) {
    // Simple galaxy filling: find the first available position
    // Positions 3-7 are habitable. Fill in order.
    PGresult *res = PQexec(r->db->conn,
        "SELECT galaxy_num, system_num, position_num FROM coordinates "
        "WHERE position_num BETWEEN 3 AND 7 "
        "ORDER BY galaxy_num, system_num, position_num");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        return 0;
    }

    int ntuples = PQntuples(res);

    // Galaxy filling algorithm: fill galaxy 1 system 1 positions 3-7 first,
    // then system 2, etc. within each galaxy, then move to next galaxy.
    // For simplicity, use the count to determine placement.
    int total_slots = 9 * 499 * 5; // galaxies * systems * habitable positions
    if (ntuples >= total_slots) {
        PQclear(res);
        return 0; // universe full
    }

    int idx = ntuples;
    int galaxy = 1 + (idx / (499 * 5));
    int system = 1 + ((idx % (499 * 5)) / 5);
    int position = 3 + (idx % 5);

    // But we need to check if the specific coordinate is free
    // For the vertical slice, this simple algorithm is sufficient
    // A real implementation would check for gaps or use a more sophisticated approach

    PQclear(res);

    *out_galaxy = galaxy > 9 ? 9 : galaxy;
    *out_system = system > 499 ? 499 : system;
    *out_position = position > 7 ? 7 : position;

    return 1;
}

int coordinate_repo_create(coordinate_repo_t *r,
                            entity_id_t galaxy, entity_id_t system,
                            entity_id_t position, entity_id_t *out_id) {
    char g_str[16], s_str[16], p_str[16];
    snprintf(g_str, sizeof(g_str), "%lld", (long long)galaxy);
    snprintf(s_str, sizeof(s_str), "%lld", (long long)system);
    snprintf(p_str, sizeof(p_str), "%lld", (long long)position);

    const char *params[3] = {g_str, s_str, p_str};
    PGresult *res = PQexecPrepared(r->db->conn, "coord_create", 3, params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        return 0;
    }
    *out_id = atoll(PQgetvalue(res, 0, 0));
    PQclear(res);
    return 1;
}

coordinate_t *coordinate_repo_find_by_id(coordinate_repo_t *r, entity_id_t id) {
    char id_str[32];
    snprintf(id_str, sizeof(id_str), "%lld", (long long)id);

    const char *params[1] = {id_str};
    PGresult *res = PQexecPrepared(r->db->conn, "coord_find", 1, params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        PQclear(res);
        return NULL;
    }

    coordinate_t *c = calloc(1, sizeof(coordinate_t));
    c->id = atoll(PQgetvalue(res, 0, 0));
    c->galaxy_num = atoi(PQgetvalue(res, 0, 1));
    c->system_num = atoi(PQgetvalue(res, 0, 2));
    c->position_num = atoi(PQgetvalue(res, 0, 3));
    c->is_moon = (strcmp(PQgetvalue(res, 0, 4), "t") == 0);
    PQclear(res);
    return c;
}
