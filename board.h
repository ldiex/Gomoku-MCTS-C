//
// Created by diex on 1/5/2024.
//

#ifndef GOMOKU_MCTS_C_BOARD_H
#define GOMOKU_MCTS_C_BOARD_H

// Define the Board struct
typedef struct {
    int width, height;
    int n_in_row; // How many pieces in a row to win, default 5
    int **states; // An array of square states
    int current_player;
    int *moves_available; // Dynamic array of available moves
    int moves_available_count; // The number of available moves
    int last_move; // The last move made, -1 if no move has been made
} Board;

// Initialize the board
void board_init(Board *b, int start_player, int width, int height, int n_in_row) {
    if (width < n_in_row || height < n_in_row) {
        printf("Board width and height cannot be less than %d.\n", n_in_row);
        exit(1);
    }

    b->width = width;
    b->height = height;
    b->n_in_row = n_in_row;
    b->current_player = start_player;

    // Initialize the moves available with 0,1, 2, ..., width * height - 1
    b->moves_available = (int*)malloc(width * height * sizeof(int));
    for (int i = 0; i < width * height; ++i) {
        b->moves_available[i] = i;
    }
    b->moves_available_count = width * height;
    // Initialize the states with -1, which means an empty square
    b->states = (int**)malloc(width * sizeof(int*));
    for (int i = 0; i < width; ++i) {
        b->states[i] = (int*)malloc(height * sizeof(int));
        for (int j = 0; j < height; ++j) {
            b->states[i][j] = -1;
        }
    }
    // Initialize the last move with -1, which means no move has been made
    b->last_move = -1;
}

// Free the memory allocated for the board
void board_free(Board *b) {
    free(b->moves_available);
    for (int i = 0; i < b->width; ++i) {
        free(b->states[i]);
    }
    free(b->states);
}

// Copy the board
void board_copy(Board *b, Board *b_copy) {
    board_init(b_copy, b->current_player, b->width, b->height, b->n_in_row);
    for (int i = 0; i < b->width; ++i) {
        for (int j = 0; j < b->height; ++j) {
            b_copy->states[i][j] = b->states[i][j];
        }
    }
    for (int i = 0; i < b->width * b->height; ++i) {
        b_copy->moves_available[i] = b->moves_available[i];
    }
    b_copy->moves_available_count = b->moves_available_count;
    b_copy->last_move = b->last_move;
}

// Convert a move to a location on the board
void board_move_to_location(Board *b, int move, int *x, int *y) {
    *x = move % b->width;
    *y = move / b->width;
}

// Convert a location on the board to a move
void board_location_to_move(Board *b, int x, int y, int *move) {
    *move = y * b->width + x;
    //if the move is invalid, return -1
    if (x < 0 || x >= b->width || y < 0 || y >= b->height) {
        *move = -1;
    }
    //if the move is not available, return -1
    if (b->moves_available[*move] == -1) {
        *move = -1;
    }
}

// Place a piece on the board
void board_do_move(Board *b, int move) {
    int x, y;
    board_move_to_location(b, move, &x, &y);
    b->states[x][y] = b->current_player;
    // Remove the move from the moves available
    b->moves_available[move] = -1;
    // Update moves_available_count
    b->moves_available_count -= 1;
    // Update the player
    b->current_player = 1 - b->current_player;
    // Update the last move
    b->last_move = move;
}

//Check forbidden moves
int board_check_forbidden(Board *b, int move) {
    int x, y;
    board_move_to_location(b, move, &x, &y);
    // Check Overline Forbidden Move
    int count = 1;
    int i = 1;
    while (x + i < b->width && b->states[x + i][y] == b->current_player) {
        ++count;
        ++i;
    }
    i = 1;
    while (x - i >= 0 && b->states[x - i][y] == b->current_player) {
        ++count;
        ++i;
    }
    if (count >= b->n_in_row) {
        return 1;
    }
    // Check Double Three Forbidden Move

}

//Check if the game is ended and return the winner
void board_check_end(Board *b, int *is_end, int *winner) {
    *is_end = 0;
    *winner = -1;
    // if the last move is -1, no move has been made
    if (b->last_move == -1) {
        return;
    }
    // Check if the board is full
    int is_full = 1;
    for (int i = 0; i < b->width * b->height; ++i) {
        if (b->moves_available[i] != -1) {
            is_full = 0;
            break;
        }
    }
    if (is_full) {
        *is_end = 1;
        *winner = -1;
        return;
    }
    // Check if there is a winner
    int x, y;
    board_move_to_location(b, b->last_move, &x, &y);
    int player = b->states[x][y];
    // Check horizontal
    int count = 1;
    int i = 1;
    while (x + i < b->width && b->states[x + i][y] == player) {
        ++count;
        ++i;
    }
    i = 1;
    while (x - i >= 0 && b->states[x - i][y] == player) {
        ++count;
        ++i;
    }
    if (count >= b->n_in_row) {
        *is_end = 1;
        *winner = player;
        return;
    }
    // Check vertical
    count = 1;
    i = 1;
    while (y + i < b->height && b->states[x][y + i] == player) {
        ++count;
        ++i;
    }
    i = 1;
    while (y - i >= 0 && b->states[x][y - i] == player) {
        ++count;
        ++i;
    }
    if (count >= b->n_in_row) {
        *is_end = 1;
        *winner = player;
        return;
    }
    // Check diagonal
    count = 1;
    i = 1;
    while (x + i < b->width && y + i < b->height && b->states[x + i][y + i] == player) {
        ++count;
        ++i;
    }
    i = 1;
    while (x - i >= 0 && y - i >= 0 && b->states[x - i][y - i] == player) {
        ++count;
        ++i;
    }
    if (count >= b->n_in_row) {
        *is_end = 1;
        *winner = player;
        return;
    }
    // Check anti-diagonal
    count = 1;
    i = 1;
    while (x + i < b->width && y - i >= 0 && b->states[x + i][y - i] == player) {
        ++count;
        ++i;
    }
    i = 1;
    while (x - i >= 0 && y + i < b->height && b->states[x - i][y + i] == player) {
        ++count;
        ++i;
    }
    if (count >= b->n_in_row) {
        *is_end = 1;
        *winner = player;
        return;
    }
}

//Draw the board and show game info
void board_draw_board(Board *b, int player1, int player2) {
    printf("Player 1: %d with X\n", player1);
    printf("Player 2: %d with O\n", player2);
    printf("\n");
    for (int i = 0; i < b->width; ++i) {
        printf("%d ", i);
    }
    printf("\n");
    for (int i = 0; i < b->height; ++i) {
        for (int j = 0; j < b->width; ++j) {
            if (b->states[j][i] == -1) {
                printf(". ");
            } else if (b->states[j][i] == player1) {
                printf("X ");
            } else {
                printf("O ");
            }
        }
        printf("%d\n", i);
    }
    printf("\n");
}

#endif //GOMOKU_MCTS_C_BOARD_H
