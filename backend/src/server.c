#include "server.h"
#include "router.h"
#include "json.h"
#include "picohttpparser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

extern router_t *g_router;

static const char *status_text(int code) {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        default:  return "OK";
    }
}

static void on_write(uv_write_t *req, int status) {
    (void)status;
    write_req_t *wreq = (write_req_t *)req;
    conn_t *conn = wreq->conn;
    free(wreq);
    if (conn) conn_close(conn);
}

static void conn_write_response(conn_t *conn) {
    write_req_t *wreq = calloc(1, sizeof(write_req_t));
    if (!wreq) return;
    wreq->conn = conn;
    wreq->req.data = wreq;

    int header_len = snprintf(wreq->header, sizeof(wreq->header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "\r\n",
        conn->status_code,
        status_text(conn->status_code),
        conn->response ? conn->response_len : 0);

    wreq->bufs[0] = uv_buf_init(wreq->header, header_len);
    wreq->bufs[1] = uv_buf_init(conn->response ? conn->response : "",
                                 conn->response ? conn->response_len : 0);

    uv_write(&wreq->req, (uv_stream_t *)&conn->handle, wreq->bufs, 2, on_write);
}

static void on_close(uv_handle_t *handle) {
    conn_t *conn = (conn_t *)handle->data;
    if (conn->response) { free(conn->response); conn->response = NULL; }
    free(conn);
}

void conn_close(conn_t *conn) {
    if (!uv_is_closing((uv_handle_t *)&conn->handle)) {
        uv_close((uv_handle_t *)&conn->handle, on_close);
    }
}

static void parse_and_dispatch(conn_t *conn) {
    const char *method_str, *path_str;
    size_t method_len, path_len;
    int minor_version;
    struct phr_header headers[MAX_HEADERS];
    size_t num_headers = MAX_HEADERS;

    int ret = phr_parse_request(conn->buffer, conn->buffer_len,
                                 &method_str, &method_len,
                                 &path_str, &path_len,
                                 &minor_version, headers, &num_headers,
                                 0);

    if (ret == -2) return;
    if (ret < 0) {
        json_error(conn, 400, "bad request");
        conn_write_response(conn);
        return;
    }

    size_t mlen = method_len < sizeof(conn->method) - 1 ? method_len : sizeof(conn->method) - 1;
    memcpy(conn->method, method_str, mlen);
    conn->method[mlen] = '\0';

    size_t plen = path_len < sizeof(conn->path) - 1 ? path_len : sizeof(conn->path) - 1;
    memcpy(conn->path, path_str, plen);
    conn->path[plen] = '\0';

    conn->num_headers = 0;
    conn->user_id = 0;
    conn->user_id_valid = 0;

    for (size_t i = 0; i < num_headers && conn->num_headers < MAX_HEADERS; i++) {
        size_t nl = headers[i].name_len < MAX_HEADER_NAME_LEN - 1 ? headers[i].name_len : MAX_HEADER_NAME_LEN - 1;
        memcpy(conn->header_names[conn->num_headers], headers[i].name, nl);
        conn->header_names[conn->num_headers][nl] = '\0';

        size_t vl = headers[i].value_len < MAX_HEADER_VALUE_LEN - 1 ? headers[i].value_len : MAX_HEADER_VALUE_LEN - 1;
        memcpy(conn->header_values[conn->num_headers], headers[i].value, vl);
        conn->header_values[conn->num_headers][vl] = '\0';

        if (strcasecmp(conn->header_names[conn->num_headers], "X-User-ID") == 0) {
            conn->user_id = atoll(conn->header_values[conn->num_headers]);
            conn->user_id_valid = 1;
        }

        conn->num_headers++;
    }

    // Body starts after the parsed request (ret = headers + body start)
    const char *body_ptr = conn->buffer + ret;
    size_t remaining = conn->buffer_len - ret;

    conn->body_len = remaining < MAX_BODY ? remaining : MAX_BODY;
    if (conn->body_len > 0) {
        memcpy(conn->body, body_ptr, conn->body_len);
        conn->body[conn->body_len] = '\0';
    } else {
        conn->body[0] = '\0';
    }

    int dispatched = router_dispatch(g_router, conn->method, conn->path, conn);
    if (!dispatched) {
        json_error(conn, 404, "not found");
    }

    conn_write_response(conn);
}

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    (void)handle;
    (void)suggested_size;
    buf->base = malloc(65536);
    buf->len = 65536;
}

static void on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
    conn_t *conn = (conn_t *)client->data;

    if (nread <= 0) {
        free(buf->base);
        if (nread < 0) conn_close(conn);
        return;
    }

    size_t remaining = MAX_BUFFER - conn->buffer_len;
    size_t to_copy = (size_t)nread < remaining ? (size_t)nread : remaining;
    if (to_copy > 0) {
        memcpy(conn->buffer + conn->buffer_len, buf->base, to_copy);
        conn->buffer_len += to_copy;
    }
    free(buf->base);

    parse_and_dispatch(conn);
}

static void on_connection(uv_stream_t *server, int status) {
    if (status < 0) return;

    conn_t *conn = calloc(1, sizeof(conn_t));
    if (!conn) return;

    server_ctx_t *ctx = (server_ctx_t *)server->data;
    conn->db = ctx->db;

    uv_tcp_init(ctx->loop, &conn->handle);
    conn->handle.data = conn;

    if (uv_accept(server, (uv_stream_t *)&conn->handle) != 0) {
        conn_close(conn);
        return;
    }

    uv_read_start((uv_stream_t *)&conn->handle, alloc_buffer, on_read);
}

int server_start(uv_loop_t *loop, void *db, const char *host, int port) {
    server_ctx_t *ctx = calloc(1, sizeof(server_ctx_t));
    if (!ctx) return -1;

    ctx->loop = loop;
    ctx->db = db;

    uv_tcp_init(loop, &ctx->tcp_server);
    ctx->tcp_server.data = ctx;

    struct sockaddr_in addr;
    uv_ip4_addr(host, port, &addr);

    uv_tcp_bind(&ctx->tcp_server, (const struct sockaddr *)&addr, 0);

    int r = uv_listen((uv_stream_t *)&ctx->tcp_server, 128, on_connection);
    if (r) {
        fprintf(stderr, "listen error: %s\n", uv_strerror(r));
        return r;
    }

    printf("server listening on %s:%d\n", host, port);
    return 0;
}
