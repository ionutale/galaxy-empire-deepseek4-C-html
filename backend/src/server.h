#ifndef SERVER_H
#define SERVER_H

#include <uv.h>
#include <stdint.h>

#define MAX_HEADERS 64
#define MAX_HEADER_NAME_LEN 64
#define MAX_HEADER_VALUE_LEN 256
#define MAX_PATH 2048
#define MAX_BODY 65536
#define MAX_BUFFER 131072

typedef struct conn {
    uv_tcp_t handle;

    char method[8];
    char path[MAX_PATH];

    char header_names[MAX_HEADERS][MAX_HEADER_NAME_LEN];
    char header_values[MAX_HEADERS][MAX_HEADER_VALUE_LEN];
    int num_headers;

    char body[MAX_BODY];
    size_t body_len;

    int64_t user_id;
    int user_id_valid;

    char path_param[64];
    int has_path_param;

    char *response;
    size_t response_len;
    int status_code;

    char buffer[MAX_BUFFER];
    size_t buffer_len;

    void *db;
} conn_t;

typedef struct {
    uv_loop_t *loop;
    uv_tcp_t tcp_server;
    void *db;
} server_ctx_t;

typedef struct {
    uv_write_t req;
    conn_t *conn;
    char header[512];
    uv_buf_t bufs[2];
} write_req_t;

int server_start(uv_loop_t *loop, void *db, const char *host, int port);
void conn_close(conn_t *conn);

#endif
