#ifndef ROUTER_H
#define ROUTER_H

#include "server.h"

#define MAX_ROUTES 128
#define MAX_PATH_LEN 256

typedef struct conn conn_t;

typedef void (*route_handler_t)(conn_t *conn, void *db);

typedef struct {
    char method[8];
    char pattern[MAX_PATH_LEN];
    route_handler_t handler;
    int has_param;
} route_t;

typedef struct {
    route_t routes[MAX_ROUTES];
    int num_routes;
    void *db;
} router_t;

void router_init(router_t *r, void *db);
void router_register(router_t *r, const char *method, const char *pattern, route_handler_t handler);
int router_dispatch(router_t *r, const char *method, const char *path, conn_t *conn);

#endif
