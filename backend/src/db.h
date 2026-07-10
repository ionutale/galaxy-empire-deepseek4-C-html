#ifndef DB_H
#define DB_H

#include <libpq-fe.h>
#include <stdint.h>

#define MAX_PREPARED_STMTS 256
#define STMT_NAME_LEN 64

typedef struct {
    PGconn *conn;
    char stmt_names[MAX_PREPARED_STMTS][STMT_NAME_LEN];
    int num_stmts;
} db_t;

db_t *db_init(const char *conninfo);
int db_prepare(db_t *db, const char *name, const char *query);
int db_exec(db_t *db, const char *sql);
void db_fini(db_t *db);

#endif
