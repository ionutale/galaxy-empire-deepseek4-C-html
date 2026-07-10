#include "json.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void json_response(conn_t *conn, int status, cJSON *root) {
    char *str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    conn->status_code = status;
    conn->response = str;
    conn->response_len = strlen(str);
}

void json_error(conn_t *conn, int status, const char *message) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "error", message);
    json_response(conn, status, root);
}

void json_success(conn_t *conn, cJSON *data) {
    json_response(conn, 200, data);
}

cJSON *json_user(const user_t *u) {
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "id", u->id);
    cJSON_AddStringToObject(obj, "username", u->username);
    return obj;
}

cJSON *json_planet(const planet_t *p, const coordinate_t *c) {
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "id", p->id);

    if (c) {
        cJSON *coord = cJSON_CreateObject();
        cJSON_AddNumberToObject(coord, "galaxy", c->galaxy_num);
        cJSON_AddNumberToObject(coord, "system", c->system_num);
        cJSON_AddNumberToObject(coord, "position", c->position_num);
        cJSON_AddBoolToObject(coord, "isMoon", c->is_moon);
        cJSON_AddItemToObject(obj, "coordinate", coord);
    }

    cJSON_AddNumberToObject(obj, "metal", p->metal);
    cJSON_AddNumberToObject(obj, "crystal", p->crystal);
    cJSON_AddNumberToObject(obj, "gas", p->gas);
    cJSON_AddNumberToObject(obj, "energy", p->energy);

    return obj;
}
