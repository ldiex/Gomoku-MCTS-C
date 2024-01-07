//
// Created by diex on 1/5/2024.
//

#ifndef GOMOKU_MCTS_C_MCTS_H
#define GOMOKU_MCTS_C_MCTS_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "board.h"
#include "game.h"

// Define the policy_value_function function that takes in a board state
// and outputs a list of actions and the probabilities of taking these actions
void policy_value_function(Board *b, int **actions, double **action_probs, int *actions_count) {
    // Set actions to moves_available that is not -1 and initialize action_probs to 1 / moves_available_count
    *actions_count = 0;
    *action_probs = NULL;
    *actions = NULL;

    for (int i = 0; i < b->height * b->width; ++i) {
        if (b->moves_available[i] != -1) {
            // If this is an available move, add it to actions
            // allocate memory for actions and action_probs if they are NULL
            if (*actions == NULL) {
                *actions = (int*)malloc(sizeof(int));
                *action_probs = (double*)malloc(sizeof(double));
            } else {
                *actions = (int*)realloc(*actions, (*actions_count + 1) * sizeof(int));
                *action_probs = (double*)realloc(*action_probs, (*actions_count + 1) * sizeof(double));
            }
            // Set actions and action_probs
            (*actions)[*actions_count] = i;
            (*action_probs)[*actions_count] = 1.0 / b->moves_available_count;
            (*actions_count)++;
        }
    }
}

// Define the rollout policy function that takes in a board state
// and outputs a list of actions and the probabilities of taking these actions
void rollout_policy_function(Board *b, int **actions, double **action_probs, int *actions_count) {
    // Set actions to moves_available that is not -1 and initialize action_probs with random numbers
    *actions_count = 0;
    *action_probs = NULL;
    *actions = NULL;

    for (int i = 0; i < b->height * b->width; ++i) {
        if (b->moves_available[i] != -1) {
            // If this is an available move, add it to actions
            // allocate memory for actions and action_probs if they are NULL
            if (*actions == NULL) {
                *actions = (int*)malloc(sizeof(int));
                *action_probs = (double*)malloc(sizeof(double));
            } else {
                *actions = (int*)realloc(*actions, (*actions_count + 1) * sizeof(int));
                *action_probs = (double*)realloc(*action_probs, (*actions_count + 1) * sizeof(double));
            }
            // Set actions and action_probs
            (*actions)[*actions_count] = i;
            (*action_probs)[*actions_count] = (double)rand() * (double)rand() / (double)RAND_MAX / (double)RAND_MAX;
            (*actions_count)++;
        }
    }
}

// Define the nodes in the MCTS tree and its functions
typedef struct treeNode {
    struct treeNode *parent;
    struct treeNode **children;
    int *actions;
    int n_children;
    int n_visits;
    double Q;
    double u;
    double p;
} TreeNode;

void tree_node_init(TreeNode *node, TreeNode *parent, double p) {
    node->parent = parent;
    // mapping form actions to children
    node->children = NULL;
    node->actions = NULL;
    node->n_children = 0;
    node->n_visits = 0;
    node->Q = 0;
    node->u = 0;
    node->p = p;
}

void tree_node_free(TreeNode *node) {
    for (int i = 0; i < node->n_children; ++i) {
        tree_node_free(node->children[i]);
    }
    free(node->children);
    free(node->actions);
}

// Expand the tree by adding new children
void tree_node_expand(TreeNode *node, int *actions, double *action_probs, int actions_count) {
    for (int i = 0; i < actions_count; ++i) {
        // if this action is not in the children, add it
        int is_in_children = 0;
        for (int j = 0; j < node->n_children; ++j) {
            if (actions[i] == node->actions[j]) {
                is_in_children = 1;
                break;
            }
        }
        if (!is_in_children) {
            node->n_children += 1;
            node->children = (TreeNode**)realloc(node->children, node->n_children * sizeof(TreeNode*));
            node->actions = (int*)realloc(node->actions, node->n_children * sizeof(int));
            TreeNode *new_node = (TreeNode*)malloc(sizeof(TreeNode));
            tree_node_init(new_node, node, action_probs[i]);
            node->children[node->n_children - 1] = new_node;
            node->actions[node->n_children - 1] = actions[i];
        }
    }
}

// Calculate and return thw value for the current node
// It is a combination of the action value Q, and this node's prior adjusted by its visit count, u
// c_puct is a number in (0, inf) controlling the relative impact of values Q
// and prior probability P on this node's score
double tree_node_value(TreeNode *node, double c_puct) {
    node->u = c_puct * node->p * sqrt(node->parent->n_visits) / (1 + node->n_visits);
    return node->Q + node->u;
}

// Select the best child node that give maximum action value Q plus bonus u(P) and return the action and the child node
void tree_node_select(TreeNode *node, double c_puct, int *action, TreeNode **child) {
    double max_value = -1;
    for (int i = 0; i < node->n_children; ++i) {
        TreeNode *child_node = node->children[i];
        double value = tree_node_value(child_node, c_puct);
        if (value > max_value) {
            max_value = value;
            *action = node->actions[i];
            *child = child_node;
        }
        node->children[i] = child_node;
    }
}

// Update the current node from leaf evaluation
// Leaf_value is the evaluation of the current board state from the perspective of the current player
void tree_node_update(TreeNode *node, double leaf_value) {
    node->n_visits += 1;
    node->Q += (leaf_value - node->Q) / node->n_visits;
}

// Update the current node recursively
// Just like tree_node_update, but applied recursively for all ancestors
void tree_node_update_recursive(TreeNode *node, double leaf_value) {
    // If it is not the root node, this node's parent should be updated first
    if (node->parent != NULL) {
        tree_node_update_recursive(node->parent, -leaf_value);
    }
    tree_node_update(node, leaf_value);
}

// Define the MCTS class and its functions
typedef struct MCTS {
    TreeNode *root;
    double c_puct;
    int n_playout; // The number of simulations to run for each move
} MCTS;

void mcts_init(MCTS *mcts, double c_puct, int n_playout) {
    mcts->root = (TreeNode*)malloc(sizeof(TreeNode));
    tree_node_init(mcts->root, NULL, 1.0);
    mcts->c_puct = c_puct;
    mcts->n_playout = n_playout;
}

void mcts_free(MCTS *mcts) {
    free(mcts->root);
}

// Evaluate the leaf node by random rollout
// Use the rollout policy to play until the end of the game
// Get the winner and return from the perspective of the current player
// Return 1 if the current player wins,
// -1 if the opponent wins, and 0 if it is a tie
double mcts_rollout(Board *b, int round_limit) {
    int is_end, winner;
    int player = b->current_player;

    for (int i = 0; i < round_limit; ++i) {
        board_check_end(b, &is_end, &winner);
        if (is_end) {
            break;
        }

        // Get actions and action_probs from the rollout policy
        int *actions = NULL;
        double *action_probs = NULL;
        int actions_count;
        rollout_policy_function(b, &actions, &action_probs, &actions_count);

        // Choose an action with the highest probability
        int action;
        double max_prob = -1;
        for (int j = 0; j < actions_count; ++j) {
            if (action_probs[j] > max_prob) {
                max_prob = action_probs[j];
                action = actions[j];
            }
        }

        board_do_move(b, action);
        if (i == round_limit - 1) {
            printf("WARNING: Round limit reached.\n");
        }

    }

    if (winner == player) {
        return 1;
    } else if (winner == -1) {
        return 0;
    } else {
        return -1;
    }
}

// Perform n_playout simulations starting from the root node to the leaf,
// getting the leaf's value and propagating it back through its parents
void mcts_playout(MCTS *mcts, Board *b) {

    TreeNode *node = mcts->root;
    while (node->n_children != 0) {
        int action;
        TreeNode *child;
        tree_node_select(node, mcts->c_puct, &action, &child);
        board_do_move(b, action);
        node = child;
    }

    // Get actions and action_probs from the policy value function
    int *actions = NULL;
    double *action_probs = NULL;
    int actions_count;

    policy_value_function(b, &actions, &action_probs, &actions_count);

    // If the game is not ended, expand the tree
    int is_end, winner;
    board_check_end(b, &is_end, &winner);
    if (!is_end) {
        tree_node_expand(node, actions, action_probs, actions_count);
    }

    // Update the leaf node recursively
    double leaf_value;
    leaf_value = mcts_rollout(b, 1000);

    // update value and visit count of nodes in this traversal with -leaf_value
    // because it is from the perspective of the other player
    tree_node_update_recursive(node, -leaf_value);
}

// Run all playouts sequentially and return the most visited action
void mcts_get_action(MCTS *mcts, Board *b, int *action) {
    for (int i = 0; i < mcts->n_playout; ++i) {
        Board b_copy;
        board_copy(b, &b_copy);
        mcts_playout(mcts, &b_copy);
        board_free(&b_copy);
    }

    // Choose the action with the highest visit count
    int max_n_visits = -1;
    for (int i = 0; i < mcts->root->n_children; ++i) {
        TreeNode *child = mcts->root->children[i];
        if (child->n_visits > max_n_visits) {
            max_n_visits = child->n_visits;
            *action = mcts->root->actions[i];
        }
    }
}

// Step forward in the tree, keeping everything we already know about the subtree
void mcts_update_with_move(MCTS *mcts, int last_move) {
    // If the last move is a child of the root, set the root to the child
    int is_child = 0;
    for (int i = 0; i < mcts->root->n_children; ++i) {
        if (mcts->root->actions[i] == last_move) {
            mcts->root = mcts->root->children[i];
            mcts->root->parent = NULL;
            is_child = 1;
            break;
        }
    }
    // If the last move is not a child of the root, set the root to a new node
    if (!is_child) {
        tree_node_free(mcts->root);
        TreeNode *new_root = (TreeNode*)malloc(sizeof(TreeNode));
        tree_node_init(new_root, NULL, 1.0);
        mcts->root = new_root;
    }
}
#endif //GOMOKU_MCTS_C_MCTS_H
