#include "game_loop.h"
#include "db.h"
#include <stdio.h>

static void on_resource_tick(uv_timer_t *handle) {
    game_loop_t *gl = (game_loop_t *)handle->data;
    (void)gl;
    printf("resource tick\n");
}

static void on_fleet_arrival(uv_timer_t *handle) {
    game_loop_t *gl = (game_loop_t *)handle->data;
    (void)gl;
    printf("fleet arrival check\n");
}

static void on_power_rank(uv_timer_t *handle) {
    game_loop_t *gl = (game_loop_t *)handle->data;
    (void)gl;
    printf("power rank recalculation\n");
}

void game_loop_init(game_loop_t *gl, uv_loop_t *loop, void *db) {
    gl->loop = loop;
    gl->db = db;

    uv_timer_init(loop, &gl->resource_tick_timer);
    gl->resource_tick_timer.data = gl;

    uv_timer_init(loop, &gl->fleet_arrival_timer);
    gl->fleet_arrival_timer.data = gl;

    uv_timer_init(loop, &gl->power_rank_timer);
    gl->power_rank_timer.data = gl;
}

void game_loop_start(game_loop_t *gl) {
    uv_timer_start(&gl->resource_tick_timer, on_resource_tick, 60000, 60000);
    uv_timer_start(&gl->fleet_arrival_timer, on_fleet_arrival, 15000, 15000);
    uv_timer_start(&gl->power_rank_timer, on_power_rank, 600000, 600000);
    printf("game loop started\n");
}

void game_loop_stop(game_loop_t *gl) {
    uv_timer_stop(&gl->resource_tick_timer);
    uv_timer_stop(&gl->fleet_arrival_timer);
    uv_timer_stop(&gl->power_rank_timer);
}
