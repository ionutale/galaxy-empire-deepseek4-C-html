#include "router.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void router_init(router_t *r, void *db) {
    r->num_routes = 0;
    r->db = db;
}

void router_register(router_t *r, const char *method, const char *pattern, route_handler_t handler) {
    if (r->num_routes >= MAX_ROUTES) return;

    route_t *route = &r->routes[r->num_routes];
    strncpy(route->method, method, sizeof(route->method) - 1);
    strncpy(route->pattern, pattern, sizeof(route->pattern) - 1);
    route->handler = handler;
    route->has_param = (strchr(pattern, ':') != NULL);
    r->num_routes++;
}

int router_dispatch(router_t *r, const char *method, const char *path, conn_t *conn) {
    for (int i = 0; i < r->num_routes; i++) {
        route_t *rt = &r->routes[i];

        if (strcmp(rt->method, method) != 0) continue;

        if (rt->has_param) {
            const char *colon = strchr(rt->pattern, ':');
            if (!colon) continue;

            size_t prefix_len = colon - rt->pattern;
            if (strncmp(path, rt->pattern, prefix_len) != 0) continue;

            const char *param_start = path + prefix_len;
            if (*param_start == '\0') continue;

            const char *slash = strchr(param_start, '/');
            size_t param_len = slash ? (size_t)(slash - param_start) : strlen(param_start);
            if (param_len >= sizeof(conn->path_param)) param_len = sizeof(conn->path_param) - 1;

            memcpy(conn->path_param, param_start, param_len);
            conn->path_param[param_len] = '\0';
            conn->has_path_param = 1;

            rt->handler(conn, r->db);
            return 1;
        }

        if (strcmp(path, rt->pattern) == 0) {
            rt->handler(conn, r->db);
            return 1;
        }
    }

    return 0;
}
