//
// Created by diex on 1/5/2024.
//

#ifndef GOMOKU_MCTS_C_GAME_H
#define GOMOKU_MCTS_C_GAME_H

#include <stdio.h>
#include <stdlib.h>

#include "board.h"
#include "mcts_player.h"

// Get player actions
void game_get_action(Board *b, int *move) {
    printf("Player %d's turn.\n", b->current_player);
    printf("Enter your move (format: x y): ");
    int x, y;
    scanf("%d %d", &x, &y);
    board_location_to_move(b, x, y, move);
    // If the move is invalid, ask for another move
    while (*move == -1) {
        printf("Invalid move. Enter your move (format: x y): ");
        scanf("%d %d", &x, &y);
        board_location_to_move(b, x, y, move);
    }
}

//Draw the board and show game info
void game_draw_board(Board *b, int player1, int player2) {
    printf("Player 1: %d with X\n", player1);
    printf("Player 2: %d with O\n", player2);
    printf("\n");

    // Calculate the number of digits in the largest index
    int maxDigits = 1;
    int maxValue = (b->width > b->height) ? b->width : b->height;
    while (maxValue /= 10) maxDigits++;

    // Print column headers with appropriate spacing
    for (int i = 0; i < b->width; ++i) {
        printf("%-*d ", maxDigits, i);
    }
    printf("\n");

    for (int i = 0; i < b->height; ++i) {
        for (int j = 0; j < b->width; ++j) {
            if (b->states[j][i] == -1) {
                printf("%-*s ", maxDigits, ".");
            } else if (b->states[j][i] == player1) {
                printf("%-*s ", maxDigits, "X");
            } else {
                printf("%-*s ", maxDigits, "O");
            }
        }
        printf("%-*d\n", maxDigits, i);
    }
    printf("\n");
}

//start a game between two human players
void game_start_human(Board *b, int start_player, int is_show_board) {
    int player1, player2;
    if (start_player == 0) {
        player1 = 0;
        player2 = 1;
    } else {
        player1 = 1;
        player2 = 0;
    }
    if (is_show_board) {
        game_draw_board(b, 0, 1);
    }
    while (1) {
        int move;
        game_get_action(b, &move);
        board_do_move(b, move);
        if (is_show_board) {
            game_draw_board(b, player1, player2);
        }
        int is_end = 0, winner = -1;
        board_check_end(b, &is_end, &winner);
        if (is_end) {
            if (winner == -1) {
                printf("Game end. Tie.\n");
            } else {
                printf("Game end. Winner is player %d.\n", winner);
            }
            break;
        }
    }
}

// start a game between a human and an MCTS player
void game_start_human_vs_mcts(Board *b, int start_player, int is_show_board, int c_puct, int n_playout) {
    int player1, player2;
    player1 = 0;
    player2 = 1;


    MCTSPlayer mcts_player;
    mcts_player_init(&mcts_player, c_puct, n_playout);

    if (is_show_board) {
        game_draw_board(b, player1, player2);
    }

    while (1) {
        int move;
        if (b->current_player == player1) {
            game_get_action(b, &move);
        } else {
            mcts_player_get_action(&mcts_player, b, &move);
        }
        board_do_move(b, move);
        if (is_show_board) {
            game_draw_board(b, player1, player2);
        }
        int is_end = 0, winner = -1;
        board_check_end(b, &is_end, &winner);
        if (is_end) {
            if (winner == -1) {
                printf("Game end. Tie.\n");
            } else {
                printf("Game end. Winner is player %d.\n", winner);
            }
            break;
        }
    }
    mcts_player_free(&mcts_player);
}

#endif //GOMOKU_MCTS_C_GAME_H

