#include "game.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

static int seeded = 0;

void game_init(Game* g) {
    g->state.stones = 10;
    g->state.target = 50;
    g->state.current = PLAYER1;
    g->state.mode = VS_FRIEND;
    g->state.over = 0;
    g->state.winner = NONE;
    if (!seeded) {
        srand((unsigned)time(NULL));
        seeded = 1;
    }
}

void game_new(Game* g, GameMode m, int init, int tgt) {
    g->state.mode = m;
    g->state.stones = init;
    g->state.target = tgt;
    g->state.current = PLAYER1;
    g->state.over = 0;
    g->state.winner = NONE;
}

int game_move(Game* g, int act, int val) {
    if (g->state.over) return 0;
    
    int new = g->state.stones;
    if (act == 1) {
        if (val < 1 || val > 5) return 0;
        new = g->state.stones + val;
    } else if (act == 2) {
        new = g->state.stones * 2;
    } else {
        return 0;
    }
    
    if (new >= g->state.target) {
        g->state.stones = new;
        g->state.over = 1;
        g->state.winner = g->state.current;
        return 1;
    }
    
    g->state.stones = new;
    g->state.current = g->state.current == PLAYER1 ? PLAYER2 : PLAYER1;
    return 1;
}

static int find_move(int stones, int target) {
    // Выигрыш за один ход
    for (int i = 1; i <= 5; i++) {
        if (stones + i >= target) return i;
    }
    if (stones * 2 >= target) return -1;
    
    // Стратегический ход
    for (int i = 5; i >= 1; i--) {
        int next = stones + i;
        if (next < target && next < target - 5) {
            int danger = 0;
            for (int j = 1; j <= 5; j++) {
                if (next + j >= target || next * 2 >= target) {
                    danger = 1;
                    break;
                }
            }
            if (!danger) return i;
        }
    }
    
    if (stones * 2 < target && stones * 2 < target - 5) return -1;
    return (rand() % 5) + 1;
}

int game_computer_move(Game* g) {
    if (g->state.over || g->state.mode != VS_COMPUTER || g->state.current != PLAYER2) {
        return 0;
    }
    
    int move = find_move(g->state.stones, g->state.target);
    return move > 0 ? game_move(g, 1, move) : game_move(g, 2, 0);
}

void game_json(Game* g, char* buf, int size) {
    snprintf(buf, size,
        "{\"s\":%d,\"t\":%d,\"c\":%d,\"m\":%d,\"o\":%s,\"w\":%d}",
        g->state.stones, g->state.target, (int)g->state.current,
        (int)g->state.mode, g->state.over ? "true" : "false", (int)g->state.winner
    );
}

