#include "game.h"

int main() {
    srand((unsigned)time(NULL));
    Board gameBoard;
    int width = 9, height = 9, n_in_row = 5;
    int start_player = 1;
    board_init(&gameBoard, start_player, width, height, n_in_row);
    // game_start_human(&gameBoard, start_player, 1);
    int c_puct = 5, n_playout = 10000;
    game_start_human_vs_mcts(&gameBoard, start_player, 1, c_puct, n_playout);
    board_free(&gameBoard);
    return 0;
}
