# Galaxy Empire (C Backend)

OGame-style space strategy game rebuilt in C for performance and minimal memory footprint.

**Stack:** C (libuv + picohttpparser + libpq + cJSON) | PostgreSQL 17 | nginx/OpenResty (JWT auth) | Svelte 5 SPA

## Quick Start

```bash
docker compose up -d
```

- **Game:** http://localhost
- **API:** http://localhost/api (via nginx with JWT)
- **DB:** localhost:5432

## Architecture

| Layer | Tech | Role |
|-------|------|------|
| Reverse Proxy | nginx/OpenResty | JWT validation, static files, rate limiting |
| API Server | C (libuv event loop) | Game logic, REST endpoints |
| Database | PostgreSQL 17 | Data persistence |
| Frontend | Svelte 5 SPA | Browser UI |

## API Routes (vertical slice)

| Method | Path | Auth | Handler |
|--------|------|------|---------|
| POST | /api/auth/register | No | handle_register |
| POST | /api/auth/login | No | handle_login |
| GET | /api/me | JWT | handle_me |
| GET | /api/planets | JWT | handle_planets_list |
| GET | /api/planets/:id | JWT | handle_planets_get |
| GET | /health | No | handle_health |

## Building

```bash
# Backend only
cd backend && make

# Full stack
docker compose build && docker compose up -d
```

## Vertical Slice

Current implementation covers: auth, planet dashboard, resource tick. More features from the Go original to be ported incrementally.
