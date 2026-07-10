#ifndef JSON_H
#define JSON_H

#include "server.h"
#include "domain/types.h"
#include <cJSON.h>

void json_error(conn_t *conn, int status, const char *message);
void json_success(conn_t *conn, cJSON *data);
cJSON *json_planet(const planet_t *p, const coordinate_t *c);
cJSON *json_user(const user_t *u);
void json_response(conn_t *conn, int status, cJSON *root);

#endif
