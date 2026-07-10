#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include <libpq-fe.h>
#include <dirent.h>

#include "server.h"
#include "router.h"
#include "json.h"
#include "db.h"
#include "game_loop.h"
#include "handler/handler.h"
#include "service.h"

const char *g_jwt_secret = NULL;
router_t *g_router = NULL;

static uv_loop_t *g_loop = NULL;
static game_loop_t *g_game_loop = NULL;

static void on_signal(uv_signal_t *handle, int signum) {
    (void)signum;
    printf("\nshutting down...\n");
    if (g_game_loop) game_loop_stop(g_game_loop);
    uv_stop(g_loop);
    uv_signal_stop(handle);
}

static void handle_health(conn_t *conn, void *db) {
    (void)db;
    json_success(conn, cJSON_CreateObject());
}

static int exec_sql(db_t *db, const char *sql) {
    PGresult *res = PQexec(db->conn, sql);
    int ok = (PQresultStatus(res) == PGRES_COMMAND_OK ||
              PQresultStatus(res) == PGRES_TUPLES_OK);
    PQclear(res);
    return ok;
}

static int run_migrations(db_t *db, const char *dir_path) {
    DIR *dir = opendir(dir_path);
    if (!dir) { fprintf(stderr, "cannot open migrations dir: %s\n", dir_path); return 0; }

    char files[256][256];
    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && count < 256) {
        if (entry->d_type == DT_REG) {
            const char *ext = strrchr(entry->d_name, '.');
            if (ext && strcmp(ext, ".sql") == 0) {
                strncpy(files[count], entry->d_name, sizeof(files[0]) - 1);
                count++;
            }
        }
    }
    closedir(dir);

    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - i - 1; j++)
            if (strcmp(files[j], files[j + 1]) > 0) {
                char tmp[sizeof(files[0])];
                strncpy(tmp, files[j], sizeof(tmp) - 1);
                strncpy(files[j], files[j + 1], sizeof(files[0]) - 1);
                strncpy(files[j + 1], tmp, sizeof(files[0]) - 1);
            }

    for (int i = 0; i < count; i++) {
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", dir_path, files[i]);
        FILE *f = fopen(path, "r");
        if (!f) { fprintf(stderr, "cannot open: %s\n", path); return 0; }
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        if (fsize <= 0) { fclose(f); continue; }
        fseek(f, 0, SEEK_SET);
        char *sql = malloc(fsize + 1);
        if (fread(sql, 1, fsize, f) != (size_t)fsize) { free(sql); fclose(f); return 0; }
        sql[fsize] = '\0';
        fclose(f);

        printf("migration: %s (%ld bytes)\n", files[i], fsize);

        char *p = sql;
        char *end = sql + fsize;
        while (p < end) {
            while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') p++;
            if (*p == '\0') break;
            char *stmt_end = p;
            while (stmt_end < end && *stmt_end != ';') stmt_end++;
            if (stmt_end >= end) break;
            stmt_end++;
            char saved = *stmt_end;
            *stmt_end = '\0';

            PGresult *res = PQexec(db->conn, p);
            if (PQresultStatus(res) != PGRES_COMMAND_OK &&
                PQresultStatus(res) != PGRES_TUPLES_OK) {
                fprintf(stderr, "migration %s failed at: %.100s\n%s\n",
                        files[i], p, PQerrorMessage(db->conn));
                PQclear(res);
                free(sql);
                return 0;
            }
            PQclear(res);
            *stmt_end = saved;
            p = stmt_end;
        }
        free(sql);
    }
    printf("migrations complete (%d files)\n", count);
    return 1;
}

int main(void) {
    const char *db_host = getenv("DB_HOST") ? getenv("DB_HOST") : "localhost";
    const char *db_port = getenv("DB_PORT") ? getenv("DB_PORT") : "5432";
    const char *db_user = getenv("DB_USER") ? getenv("DB_USER") : "postgres";
    const char *db_pass = getenv("DB_PASS") ? getenv("DB_PASS") : "postgres";
    const char *db_name = getenv("DB_NAME") ? getenv("DB_NAME") : "galaxy_empire";
    const char *migrations_dir = getenv("MIGRATIONS_DIR") ? getenv("MIGRATIONS_DIR") : "/app/migrations";
    const char *listen_host = getenv("LISTEN_HOST") ? getenv("LISTEN_HOST") : "0.0.0.0";
    const char *listen_port_str = getenv("LISTEN_PORT") ? getenv("LISTEN_PORT") : "8080";
    g_jwt_secret = getenv("JWT_SECRET") ? getenv("JWT_SECRET") : "dev-secret-change-in-production";
    int listen_port = atoi(listen_port_str);

    char conninfo[512];
    snprintf(conninfo, sizeof(conninfo),
        "host=%s port=%s user=%s password=%s dbname=%s",
        db_host, db_port, db_user, db_pass, db_name);

    db_t *db = db_init(conninfo);
    if (!db) { fprintf(stderr, "failed to connect to database\n"); return 1; }

    if (!run_migrations(db, migrations_dir)) {
        fprintf(stderr, "migrations failed\n");
        db_fini(db);
        return 1;
    }

    if (!repos_prepare_all(db)) { fprintf(stderr, "repo prep failed\n"); db_fini(db); return 1; }

    router_t router;
    g_router = &router;
    router_init(&router, db);

    router_register(&router, "POST", "/api/auth/register", handle_register);
    router_register(&router, "POST", "/api/auth/login", handle_login);
    router_register(&router, "GET", "/api/me", handle_me);
    router_register(&router, "GET", "/api/planets", handle_planets_list);
    router_register(&router, "GET", "/api/planets/:id", handle_planets_get);
    router_register(&router, "POST", "/api/planets/:id/build", handle_build);
    router_register(&router, "POST", "/api/planets/:id/cancel", handle_cancel_build);
    router_register(&router, "GET", "/api/planets/:id/build/:building", handle_building_info);
    router_register(&router, "GET", "/api/planets/:id/shipyard", handle_shipyard);
    router_register(&router, "POST", "/api/planets/:id/produce", handle_produce);
    router_register(&router, "POST", "/api/planets/:id/cancel-production", handle_cancel_prod);
    router_register(&router, "GET", "/api/research", handle_research);
    router_register(&router, "POST", "/api/research/:tech", handle_queue_research);
    router_register(&router, "POST", "/api/research/cancel", handle_cancel_research);
    router_register(&router, "GET", "/api/research/:tech/info", handle_tech_info);
    router_register(&router, "GET", "/api/galaxy/:galaxy/:system", handle_galaxy);
    router_register(&router, "POST", "/api/fleet/send", handle_fleet_send);
    router_register(&router, "POST", "/api/fleet/:id/recall", handle_fleet_recall);
    router_register(&router, "GET", "/api/fleet", handle_fleet_list);
    router_register(&router, "GET", "/api/fleet/estimate", handle_fleet_est);
    router_register(&router, "GET", "/api/fleet/presets", handle_presets);
    router_register(&router, "POST", "/api/fleet/presets", handle_preset_create);
    router_register(&router, "DELETE", "/api/fleet/presets/:id", handle_preset_del);
    router_register(&router, "GET", "/api/reports", handle_reports);
    router_register(&router, "GET", "/api/reports/spy", handle_spy_reports);
    router_register(&router, "GET", "/api/reports/:id", handle_report);
    router_register(&router, "GET", "/api/power-rank", handle_power_rank);
    router_register(&router, "GET", "/api/commanders", handle_commanders);
    router_register(&router, "POST", "/api/commanders/hire", handle_hire);
    router_register(&router, "GET", "/api/fortress", handle_fortress);
    router_register(&router, "POST", "/api/fortress/attack", handle_fortress_attack);
    router_register(&router, "POST", "/api/fortress/claim", handle_fortress_claim);
    router_register(&router, "GET", "/api/wormhole/status", handle_wormhole);
    router_register(&router, "POST", "/api/wormhole/send", handle_wormhole_send);
    router_register(&router, "GET", "/api/eu-x/status", handle_eux);
    router_register(&router, "POST", "/api/eu-x/scan", handle_eux_scan);
    router_register(&router, "GET", "/api/galactonites", handle_galactonites);
    router_register(&router, "POST", "/api/galactonites/equip", handle_galactonite_equip);
    router_register(&router, "POST", "/api/galactonites/unequip", handle_galactonite_unequip);
    router_register(&router, "POST", "/api/galactonites/upgrade", handle_galactonite_upgrade);
    router_register(&router, "POST", "/api/galactonites/fuse", handle_galactonite_fuse);
    router_register(&router, "POST", "/api/galactonites/convert", handle_galactonite_convert);
    router_register(&router, "POST", "/api/galactonites/buy-energy", handle_galactonite_buy_energy);
    router_register(&router, "GET", "/api/galactonites/crystal-energy", handle_crystal_energy);
    router_register(&router, "GET", "/api/quests", handle_quests);
    router_register(&router, "POST", "/api/quests/claim", handle_claim_quest);
    router_register(&router, "POST", "/api/daily-gift", handle_daily_gift);
    router_register(&router, "GET", "/api/profile", handle_profile);
    router_register(&router, "POST", "/api/profile/change-password", handle_change_password);
    router_register(&router, "POST", "/api/alliance/create", handle_alliance_create);
    router_register(&router, "GET", "/api/alliance/mine", handle_alliance_mine);
    router_register(&router, "GET", "/api/alliance/:id", handle_alliance_get);
    router_register(&router, "POST", "/api/alliance/:id/leave", handle_alliance_leave);
    router_register(&router, "POST", "/api/alliance/:id/disband", handle_alliance_disband);
    router_register(&router, "POST", "/api/alliance/:id/promote/:userId", handle_alliance_promote);
    router_register(&router, "POST", "/api/alliance/:id/demote/:userId", handle_alliance_demote);
    router_register(&router, "POST", "/api/alliance/:id/bulletin", handle_alliance_bulletin);
    router_register(&router, "POST", "/api/alliance/:id/mail", handle_alliance_mail);
    router_register(&router, "GET", "/api/alliance/:id/mail", handle_alliance_mail_list);
    router_register(&router, "POST", "/api/alliance/:id/join", handle_alliance_join);
    router_register(&router, "POST", "/api/alliance/:id/invite", handle_alliance_invite);
    router_register(&router, "POST", "/api/alliance/:id/approve/:userId", handle_alliance_approve);
    router_register(&router, "POST", "/api/alliance/:id/deny/:userId", handle_alliance_deny);
    router_register(&router, "GET", "/api/alliance/:id/join-requests", handle_alliance_requests);
    router_register(&router, "POST", "/api/alliance/:id/vote-initiate", handle_alliance_vote);
    router_register(&router, "POST", "/api/alliance/:id/vote", handle_alliance_vote_cast);
    router_register(&router, "GET", "/api/alliance/:id/votes", handle_alliance_vote_active);
    router_register(&router, "GET", "/api/gef/stages", handle_gef);
    router_register(&router, "POST", "/api/gef/attack", handle_gef_attack);
    router_register(&router, "POST", "/admin/set-resources", handle_admin_set_resources);
    router_register(&router, "POST", "/admin/set-building", handle_admin_set_building);
    router_register(&router, "POST", "/admin/set-dark-matter", handle_admin_set_dm);
    router_register(&router, "POST", "/admin/set-credits", handle_admin_set_credits);
    router_register(&router, "POST", "/admin/set-research", handle_admin_set_research);
    router_register(&router, "POST", "/admin/set-fleet", handle_admin_set_fleet);
    router_register(&router, "POST", "/admin/set-defense", handle_admin_set_defense);
    router_register(&router, "GET", "/health", handle_health);

    uv_loop_t *loop = uv_default_loop();
    g_loop = loop;

    if (server_start(loop, db, listen_host, listen_port) != 0) {
        fprintf(stderr, "failed to start server\n");
        db_fini(db);
        return 1;
    }

    game_loop_t game_loop;
    g_game_loop = &game_loop;
    game_loop_init(&game_loop, loop, db);
    game_loop_start(&game_loop);

    uv_signal_t sigint, sigterm;
    uv_signal_init(loop, &sigint);
    uv_signal_start(&sigint, on_signal, SIGINT);
    uv_signal_init(loop, &sigterm);
    uv_signal_start(&sigterm, on_signal, SIGTERM);

    printf("server ready\n");
    uv_run(loop, UV_RUN_DEFAULT);

    game_loop_stop(&game_loop);
    db_fini(db);
    printf("shutdown complete\n");
    return 0;
}
