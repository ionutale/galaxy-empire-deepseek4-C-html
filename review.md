# Code Review — Galaxy Empire C Backend

**Date:** 2026-07-10
**Scope:** `backend/` — C backend (libuv server, libpq DB, handler/repo/service layers)
**Stack:** C (gcc/musl) | libuv 1.49 | libpq (PostgreSQL 17) | OpenResty + lua-resty-jwt | Alpine 3.21

---

## Findings Summary

| # | Severity | Category | Finding | File |
|---|----------|----------|---------|------|
| 1 | 🔴 Critical | Credentials | Default DB password in docker-compose | [docker-compose.yml](docker-compose.yml) |
| 2 | 🔴 Critical | Credentials | Default JWT secret in production config | [docker-compose.yml](docker-compose.yml) + [main.c](backend/src/main.c) |
| 3 | 🔴 Critical | Data integrity | Multi-step writes without transactions | [handler.c](backend/src/handler/handler.c) |
| 4 | 🔴 Critical | Data integrity | DELETE + INSERT without transaction | [fortress_repo.c](backend/src/repo/fortress_repo.c) |
| 5 | 🟠 High | Crypto | SHA-256 instead of bcrypt/argon2 for passwords | [handler.c](backend/src/handler/handler.c) |
| 6 | 🟠 High | Input validation | No username sanitization in JWT payload | [jwt.c](backend/src/jwt.c) |
| 7 | 🟠 High | Auth | Missing admin auth middleware | [main.c](backend/src/main.c) |
| 8 | 🟡 Medium | Performance | Single libpq connection — no pool | [db.c](backend/src/db.c) |
| 9 | 🟡 Medium | Resilience | Migration runner doesn't track applied versions | [main.c](backend/src/main.c) |
| 10 | 🟡 Medium | Reliability | Unchecked malloc return values | [server.c](backend/src/server.c) |
| 11 | 🟡 Medium | Data integrity | `strncpy` misuse (no null-termination guarantee) | multiple files |
| 12 | 🟡 Medium | Architecture | 60+ routes with only 128 max in router | [router.h](backend/src/router.h) |
| 13 | 🟡 Medium | Concurrency | Global mutable state (`g_router`, `g_jwt_secret`) | [main.c](backend/src/main.c) |
| 14 | 🟢 Low | Code quality | Unused `is_defense` parameter | [config.c](backend/src/config.c) |
| 15 | 🟢 Low | Code quality | Raw SQL in handler.c instead of prepared statements | [handler.c](backend/src/handler/handler.c) |
| 16 | 🟢 Low | Ops | No `restart: unless-stopped` on containers | [docker-compose.yml](docker-compose.yml) |

---

## 🔴 Critical Findings

### 1. Default DB password in docker-compose.yml

**File:** [docker-compose.yml](docker-compose.yml):6

```yaml
    environment:
      POSTGRES_PASSWORD: postgres
```

The database password `postgres` is hardcoded in version control. Anyone with access to the repo can connect to the production database if the container is exposed.

**Fix:** Use an `.env` file or Docker secrets:

```yaml
    environment:
      POSTGRES_PASSWORD: ${DB_PASSWORD:?must be set}
```

### 2. Default JWT secret in production

**File:** [docker-compose.yml](docker-compose.yml):28, [main.c](backend/src/main.c):126

```yaml
    environment:
      JWT_SECRET: change-me-in-production
```

```c
g_jwt_secret = getenv("JWT_SECRET") ? getenv("JWT_SECRET") : "dev-secret-change-in-production";
```

The JWT signing key defaults to a well-known string. An attacker who knows this can forge valid tokens for any user.

**Fix:** Remove defaults — crash at startup if `JWT_SECRET` is unset:

```c
g_jwt_secret = getenv("JWT_SECRET");
if (!g_jwt_secret) { fprintf(stderr, "JWT_SECRET not set\n"); return 1; }
```

### 3. Multi-step writes without transactions

**File:** [handler.c](backend/src/handler/handler.c):60-83

```c
user_repo_create(ur, username, hash, &uid);   // step 1
coordinate_repo_create(cr, gal, sys, pos, &cid); // step 2
planet_repo_create_home(pr, uid, cid, &pid);    // step 3
db_exec(DB, "UPDATE users SET home_planet_id=..."); // step 4
```

If the server crashes between steps 2 and 3, a coordinate is allocated but never owned by a planet. Between 3 and 4, a user exists with no home planet reference. This creates orphaned rows and inconsistent state.

**Fix:** Wrap in `BEGIN` / `COMMIT`:

```c
PQexec(DB->conn, "BEGIN");
// ... all steps ...
PQexec(DB->conn, "COMMIT");
```

### 4. DELETE + INSERT without transaction

**File:** [fortress_repo.c](backend/src/repo/fortress_repo.c):46-55

```c
int fortress_repo_set_control(fortress_repo_t *r,entity_id_t aid){
    PGresult *res=PQexecPrepared(r->db->conn,"fo_del",0,NULL,NULL,NULL,0);
    // ... if crash here, fortress_control is empty ...
    res=PQexecPrepared(r->db->conn,"fo_ins",1,p,NULL,NULL,0);
```

The fortress control table is cleared before the new control row is inserted. If the process crashes between the DELETE and INSERT, the fortress has no controlling alliance — data loss.

**Fix:** Use a single atomic UPSERT or wrap in a transaction.

---

## 🟠 High Findings

### 5. SHA-256 instead of bcrypt/argon2 for passwords

**File:** [handler.c](backend/src/handler/handler.c):55-58

```c
unsigned char sha[SHA256_DIGEST_LENGTH]; SHA256((unsigned char*)salted, strlen(salted), sha);
```

SHA-256 is designed for speed — an attacker can test billions of passwords per second with GPU hardware. Even with a per-user salt, this is not adequate for credential storage.

**Fix:** Use `libsodium`'s `crypto_pwhash_str` (argon2id) which is deliberately slow and memory-hard. Add `libsodium-dev` to the Docker build:

```c
if (crypto_pwhash_str(hash, password, strlen(password),
    crypto_pwhash_OPSLIMIT_MODERATE, crypto_pwhash_MEMLIMIT_MODERATE) != 0)
    return error;
```

### 6. No username sanitization in JWT payload

**File:** [jwt.c](backend/src/jwt.c):49-51

```c
snprintf(payload, sizeof(payload),
    "{\"user_id\":%ld,\"username\":\"%s\",\"exp\":%ld,\"iat\":%ld}",
    (long)user_id, username, (long)exp, (long)now);
```

If a username contains a double quote (`"`), backslash (`\`), or other JSON-special characters, this produces malformed JSON that breaks the JWT payload. This is exploitable during registration.

**Fix:** Use cJSON to build the payload:

```c
cJSON *p = cJSON_CreateObject();
cJSON_AddNumberToObject(p, "user_id", user_id);
cJSON_AddStringToObject(p, "username", username);
cJSON_AddNumberToObject(p, "exp", exp);
cJSON_AddNumberToObject(p, "iat", now);
char *payload = cJSON_PrintUnformatted(p);
```

### 7. Admin endpoints have no auth middleware

**File:** [main.c](backend/src/main.c):218-224

```c
router_register(&router, "POST", "/admin/set-resources", handle_admin_set_resources);
```

The `/admin/*` routes are registered without any admin-role check. Any authenticated user (with any JWT) can call them. The Go backend had an `AdminMiddleware` that checked `X-Admin-ID` header or admin IDs list.

**Fix:** Add an admin check before handler dispatch, or register admin routes behind a middleware wrapper (the router doesn't support middleware yet — add a check in each admin handler):

```c
static int is_admin(conn_t *conn) {
    return conn->user_id_valid && conn->user_id == ADMIN_USER_ID;
}
```

---

## 🟡 Medium Findings

### 8. Single libpq connection — no pool

**File:** [db.c](backend/src/db.c):10

```c
db->conn = PQconnectdb(conninfo);
 ```

The entire server shares one PostgreSQL connection. Under concurrent requests, all DB operations serialize on this single connection. A slow query blocks every other request.

**Fix:** Implement a simple connection pool (e.g., 4-8 connections) backed by `uv_queue_work`. Each request borrows a connection from the pool, uses it, and returns it.

### 9. Migration runner doesn't track applied versions

**File:** [main.c](backend/src/main.c):70-111

The migration runner blindly executes all `.sql` files in sorted order on every startup. If a migration succeeds partially and the process crashes, re-running will fail on `CREATE TABLE IF NOT EXISTS` (safe) but won't re-apply any data migrations. There's also no tracking of which version the database is at vs which files exist.

**Fix:** Create a `schema_migrations` table and only apply files that haven't been applied yet:

```sql
CREATE TABLE IF NOT EXISTS schema_migrations (version TEXT PRIMARY KEY, applied_at TIMESTAMPTZ DEFAULT NOW());
```

```c
// Before each file:
PGresult *r = PQexec(db->conn, "SELECT 1 FROM schema_migrations WHERE version='000001'");
if (PQntuples(r) > 0) { /* skip */ }
```

### 10. Unchecked malloc return values

**File:** [server.c](backend/src/server.c):141

```c
buf->base = malloc(65536);
buf->len = 65536;
```

If `malloc` returns NULL (OOM), the server will segfault when libuv tries to write to the buffer. Same pattern in `alloc_buffer` and `conn_write_response`.

**Fix:** Check for NULL:

```c
buf->base = malloc(65536);
if (!buf->base) { /* close connection, return */ }
buf->len = 65536;
```

### 11. `strncpy` misuse

**File:** Multiple repo files

```c
strncpy(out->building, PQgetvalue(res,i,1), sizeof(out->building) - 1);
```

`strncpy` does NOT null-terminate if the source is >= the buffer size. This can lead to buffer reads past the end.

**Fix:** Use `snprintf` or manually null-terminate:

```c
snprintf(out->building, sizeof(out->building), "%s", PQgetvalue(res,i,1));
```

### 12. Route table capacity too tight

**File:** [router.h](backend/src/router.h):8

```c
#define MAX_ROUTES 128
```

Currently ~65 routes are registered. The remaining 63 slots leave little room for expansion and no room for parameterized route variants.

**Fix:** Bump to `MAX_ROUTES 256` or switch to a dynamic array.

### 13. Global mutable state

**File:** [main.c](backend/src/main.c):16-20

```c
const char *g_jwt_secret = NULL;
router_t *g_router = NULL;
```

Global variables prevent the server from running multiple independent instances in the same process and make the dependency chain invisible. The server's `on_connection` callback needs the router — it currently accesses it through a global.

**Fix:** Store the router pointer in the `server_ctx_t` struct and pass it through the connection lifecycle, rather than via a global.

---

## 🟢 Low Findings

### 14. Unused `is_defense` parameter

**File:** [config.c](backend/src/config.c):177

```c
int64_t get_unit_cost_metal(const char *type, int is_defense) {
```

The `is_defense` parameter is never used — the function looks up the type name regardless. This generates a compiler warning which is treated as error with `-Werror`.

**Fix:** Remove the parameter or `(void)is_defense`.

### 15. Raw SQL in handler.c

**File:** [handler.c](backend/src/handler/handler.c):65

```c
PGresult *gres = PQexec(DB->conn, "SELECT galaxy_num, system_num, position_num FROM coordinates ...");
```

Several handler functions use `PQexec` with raw SQL strings instead of the project's prepared statement pattern. This bypasses the parameterized query approach used in the repo layer.

**Fix:** Move these queries into the appropriate repo files and use prepared statements.

### 16. Missing restart policy on containers

**File:** [docker-compose.yml](docker-compose.yml)

None of the services have a `restart` policy. If the backend or nginx crashes (e.g., OOM), they stay stopped.

**Fix:**

```yaml
  backend:
    restart: unless-stopped
  nginx:
    restart: unless-stopped
```

---

## Quick Wins (< 1 hour)

1. Add `restart: unless-stopped` to all services in docker-compose.yml
2. Bump `MAX_ROUTES` from 128 to 256
3. Remove unused `is_defense` parameters in config.c
4. Replace `strncpy` with `snprintf` in repo files (search-and-replace pattern)
5. Crash on missing `JWT_SECRET` instead of using default
6. Add `BEGIN`/`COMMIT` around the fortress_repo DELETE+INSERT pair

## Roadmap

| Effort | Items |
|--------|-------|
| **Now** (hotfixes) | #1 (default password), #2 (default JWT secret), #16 (restart policy) |
| **This week** | #4 (fortress transaction), #7 (admin auth), #6 (username sanitization) |
| **This sprint** | #3 (registration transaction), #5 (bcrypt), #10 (malloc checks) |
| **Backlog** | #8 (connection pool), #9 (migration tracking), #12 (route capacity), #13 (globals), #11 (strncpy) |
