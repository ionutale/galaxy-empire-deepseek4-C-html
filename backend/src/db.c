#include "db.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

db_t *db_init(const char *conninfo) {
    db_t *db = calloc(1, sizeof(db_t));
    if (!db) return NULL;

    db->conn = PQconnectdb(conninfo);
    if (PQstatus(db->conn) != CONNECTION_OK) {
        fprintf(stderr, "db connection failed: %s\n", PQerrorMessage(db->conn));
        db_fini(db);
        return NULL;
    }

    printf("db connected: %s\n", PQdb(db->conn));
    return db;
}

int db_prepare(db_t *db, const char *name, const char *query) {
    if (db->num_stmts >= MAX_PREPARED_STMTS) return 0;

    PGresult *res = PQprepare(db->conn, name, query, 0, NULL);
    int ok = (PQresultStatus(res) == PGRES_COMMAND_OK);
    if (!ok) {
        fprintf(stderr, "prepare '%s' failed: %s\n", name, PQerrorMessage(db->conn));
    }
    PQclear(res);

    if (ok) {
        strncpy(db->stmt_names[db->num_stmts], name, STMT_NAME_LEN - 1);
        db->num_stmts++;
    }
    return ok;
}

int db_exec(db_t *db, const char *sql) {
    PGresult *res = PQexec(db->conn, sql);
    int ok = (PQresultStatus(res) == PGRES_COMMAND_OK ||
              PQresultStatus(res) == PGRES_TUPLES_OK);
    if (!ok) {
        fprintf(stderr, "exec failed: %s\n", PQerrorMessage(db->conn));
    }
    PQclear(res);
    return ok;
}

void db_fini(db_t *db) {
    if (db) {
        if (db->conn) {
            PQfinish(db->conn);
        }
        free(db);
    }
}
