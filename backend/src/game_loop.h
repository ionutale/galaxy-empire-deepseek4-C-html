#ifndef GAME_LOOP_H
#define GAME_LOOP_H

#include <uv.h>

typedef struct {
    uv_loop_t *loop;
    uv_timer_t resource_tick_timer;
    uv_timer_t fleet_arrival_timer;
    uv_timer_t power_rank_timer;
    void *db;
} game_loop_t;

void game_loop_init(game_loop_t *gl, uv_loop_t *loop, void *db);
void game_loop_start(game_loop_t *gl);
void game_loop_stop(game_loop_t *gl);

#endif
