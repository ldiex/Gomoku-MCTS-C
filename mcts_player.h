//
// Created by diex on 1/6/2024.
//

#ifndef GOMOKU_MCTS_C_MCTS_PLAYER_H
#define GOMOKU_MCTS_C_MCTS_PLAYER_H

#include "mcts.h"

// Define the MCTS player
typedef struct {
    MCTS mcts;
} MCTSPlayer;

// Initialize the MCTS player
void mcts_player_init(MCTSPlayer *player,int c_puct, int n_playout) {
    MCTS mcts;
    mcts_init(&mcts, c_puct, n_playout);
    player->mcts = mcts;
}

// Free the memory allocated for the MCTS player
void mcts_player_free(MCTSPlayer *player) {
    mcts_free(&player->mcts);
}

// Get the MCTS player's action
void mcts_player_get_action(MCTSPlayer *player, Board *b, int *move) {
    int _move;
    mcts_get_action(&player->mcts, b, &_move);
    mcts_update_with_move(&player->mcts, -1);
    *move = _move;
}

// Reset the MCTS player
void mcts_player_reset_player(MCTSPlayer *player) {
    mcts_update_with_move(&player->mcts, -1);
}

#endif //GOMOKU_MCTS_C_MCTS_PLAYER_H
