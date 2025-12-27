#ifndef GAME_H
#define GAME_H

typedef enum { VS_FRIEND = 0, VS_COMPUTER = 1 } GameMode;
typedef enum { PLAYER1 = 0, PLAYER2 = 1, NONE = 2 } Player;

typedef struct {
    int stones;
    int target;
    Player current;
    GameMode mode;
    int over;
    Player winner;
} GameState;

typedef struct {
    GameState state;
} Game;

void game_init(Game* g);
void game_new(Game* g, GameMode m, int init, int tgt);
int game_move(Game* g, int act, int val);
int game_computer_move(Game* g);
void game_json(Game* g, char* buf, int size);

#endif

