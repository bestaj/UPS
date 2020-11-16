#include <stdlib.h>
#include "message.h"
#include "client.h"
#include "room.h"

/*          Playing board
  0 ------------- 1 ------------ 2
  |               |              |
  |    8 -------- 9 ------ 10    |
  |    |          |         |    |
  |    |     16--17--18     |    |
  |    |     |        |     |    |
  7 -- 15 -- 23      19 -- 11 -- 3
  |    |     |        |     |    |
  |    |     22--21--20     |    |
  |    |          |         |    |
  |    14 ------ 13 ------ 12    |
  |               |              |
  6 ------------- 5 ------------ 4
*/

/* Neighbours of every single game position */
int nbrs_0[] = {1, 7};
int nbrs_1[] = {0, 2, 9};
int nbrs_2[] = {1, 3};
int nbrs_3[] = {2, 4, 11};
int nbrs_4[] = {3, 5};
int nbrs_5[] = {4, 6, 13};
int nbrs_6[] = {5, 7};
int nbrs_7[] = {0, 6, 15};
int nbrs_8[] = {9, 15};
int nbrs_9[] = {1, 8, 10, 17};
int nbrs_10[] = {9, 11};
int nbrs_11[] = {3, 10, 12, 19};
int nbrs_12[] = {11, 13};
int nbrs_13[] = {5, 12, 14, 21};
int nbrs_14[] = {13, 15};
int nbrs_15[] = {7, 8, 14, 23};
int nbrs_16[] = {17, 23};
int nbrs_17[] = {9, 16, 18};
int nbrs_18[] = {17, 19};
int nbrs_19[] = {11, 18, 20};
int nbrs_20[] = {19, 21};
int nbrs_21[] = {13, 20, 22};
int nbrs_22[] = {21, 23};
int nbrs_23[] = {15, 16, 22};

int *neighbours[] = {nbrs_0, nbrs_1, nbrs_2, nbrs_3, nbrs_4, nbrs_5, nbrs_6, nbrs_7, nbrs_8, nbrs_9, nbrs_10, nbrs_11,
                     nbrs_12, nbrs_13, nbrs_14, nbrs_15, nbrs_16, nbrs_17, nbrs_18, nbrs_19, nbrs_20, nbrs_21, nbrs_22, nbrs_23};

/* Create a room (game) */
room *create_room(int number, client *player1) {
    room *new_room;
    int i;

    if (number < 0 || !player1) return NULL;

    new_room = (room *) malloc(sizeof(room));
    if (!new_room) return NULL;

    new_room->number = number;
    new_room->player1 = player1;
    new_room->room_state = FREE;
    new_room->p1_stones = 9;
    new_room->p2_stones = 9;
    new_room->p1_unset_stones = 9;
    new_room->p2_unset_stones = 9;

    for (i = 0; i < 24; i++) {
        new_room->game_positions[i] = FREE;
    }

    return new_room;
}

/* Clear the room */
void remove_room(room *room) {
    free(room);
    room = NULL;
}

/* Return the opponent */
client *get_opponent(room *room, client *player) {
    if (player->is_player1)
        return room->player2;
    else
        return room->player1;
}

/* Test if the stone is inside any mill */
int is_mill(int *game_positions, int pos_id, int player_stone) {
    int i;
    int rest_mod_8 = pos_id % 8;;

    if (pos_id % 2 == 0) { // Corners
        if (rest_mod_8 == 0) { // Left upper corner
            if (game_positions[pos_id + 1] == player_stone) {
                if (game_positions[pos_id + 2] == player_stone) {
                    return TRUE;
                }
            }
            if (game_positions[pos_id + 6] == player_stone) {
                if (game_positions[pos_id + 7] == player_stone) {
                    return TRUE;
                }
            }
        }
        else if (rest_mod_8 == 6) { // Left bottom corner
            if (game_positions[pos_id - 1] == player_stone) {
                if (game_positions[pos_id - 2] == player_stone) {
                    return TRUE;
                }
            }
            if (game_positions[pos_id + 1] == player_stone) {
                if (game_positions[pos_id - 6] == player_stone) {
                    return TRUE;
                }
            }
        }
        else { // All other corners
            if (game_positions[pos_id + 1] == player_stone) {
                if (game_positions[pos_id + 2] == player_stone) {
                    return TRUE;
                }
            }
            if (game_positions[pos_id - 1] == player_stone) {
                if (game_positions[pos_id - 2] == player_stone) {
                    return TRUE;
                }
            }
        }
    }
    else { // Middle game positions
        if (rest_mod_8 == 7) {
            if (game_positions[pos_id - 1] == player_stone) {
                if (game_positions[pos_id - 7] == player_stone) {
                    return TRUE;
                }
            }
        }
        else {
            if (game_positions[pos_id + 1] == player_stone) {
                if (game_positions[pos_id - 1] == player_stone) {
                    return TRUE;
                }
            }
        }
        if (pos_id < 8) { // Middle of large square
            if (game_positions[pos_id + 8] == player_stone) {
                if (game_positions[pos_id + 16] == player_stone) {
                    return TRUE;
                }
            }
        }
        else if (pos_id > 15) { // Middle of small square
            if (game_positions[pos_id - 8] == player_stone) {
                if (game_positions[pos_id - 16] == player_stone) {
                    return TRUE;
                }
            }
        }
        else { // Middle of middle square
            if (game_positions[pos_id + 8] == player_stone) {
                if (game_positions[pos_id - 8] == player_stone) {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

/* Test if client can take some opponents stone which is not inside mill */
int isTakeStonePossible(int *game_positions, client *player) {
    int i, opponent;

    if (player->is_player1) {
        opponent = P2;
    }
    else {
        opponent = P1;
    }

    for (i = 0; i < POSITIONS_COUNT; i++) {
        if (game_positions[i] == opponent) {
            if (!is_mill(game_positions, i, opponent)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

/* Set the stone on a specific game position */
response set_stone(room *room, client *player, int pos_id) {

    if (player->is_player1) {
        if (room->p1_unset_stones == 0)
            return TURN_ERR_CANNOT_SET;   // Client already set all his stones
    }
    else {
        if (room->p2_unset_stones == 0)
            return TURN_ERR_CANNOT_SET;
    }

    if (room->game_positions[pos_id] != FREE) {
        return TURN_ERR_FULL_POS;     // Client cannot set the stone on a full game position
    }

    if (player->is_player1) {
        room->game_positions[pos_id] = P1;
        room->p1_unset_stones--;
        if (is_mill(room->game_positions, pos_id, P1) == TRUE) {
            if (isTakeStonePossible(room->game_positions, player)) {
                return TURN_OK_MILL;
            }
            else {
                return TURN_OK_MILL_NO_TAKE;
            }
        }
    }
    else {
        room->game_positions[pos_id] = P2;
        room->p2_unset_stones--;
        if (is_mill(room->game_positions, pos_id, P2) == TRUE) {
            if (isTakeStonePossible(room->game_positions, player)) {
                return TURN_OK_MILL;
            }
            else {
                return TURN_OK_MILL_NO_TAKE;
            }
        }
    }

    return TURN_OK_NOT_MILL;
}

/* Test if position 2 is valid neighbour of position 1 */
int exist_free_neighbour(room *room, int pos1_id, int pos2_id) {
    int i, nbrs_count;

    if (room->game_positions[pos2_id] != FREE) {
        return FALSE;
    }

    if (pos1_id % 2 == 0) {
        nbrs_count = 2;
    }
    else {
        if (pos1_id > 7 && pos1_id < 16)
            nbrs_count = 4;
        else
            nbrs_count = 3;
    }

    for (i = 0; i < nbrs_count; i++) {
        if (neighbours[pos1_id][i] == pos2_id) {
            return TRUE;
        }
    }

    return FALSE;
}

/* Shift the stone from position 1 to position 2 */
response shift_stone(room *room, client *player, int pos1_id, int pos2_id) {

    if (player->is_player1) {
        if (room->p1_unset_stones != 0)
            return TURN_ERR_CANNOT_SHIFT;   // Client still has stones which have to be set before shifting
        if (room->game_positions[pos1_id] != P1)
            return TURN_ERR_MISSING_STONE; // On the position 1 is not clients stone
    }
    else {
        if (room->p2_unset_stones != 0)
            return TURN_ERR_CANNOT_SHIFT;
        if (room->game_positions[pos1_id] != P2)
            return TURN_ERR_MISSING_STONE;
    }

    if (exist_free_neighbour(room, pos1_id, pos2_id) == FALSE) {
        return TURN_ERR_WRONG_POS2; // Position 2 is not next to position 1
    }

    room->game_positions[pos1_id] = FREE;

    if (player->is_player1) {
        room->game_positions[pos2_id] = P1;
        if (is_mill(room->game_positions, pos2_id, P1) == TRUE) {
            if (isTakeStonePossible(room->game_positions, player)) {
                return TURN_OK_MILL;
            }
            else {
                return TURN_OK_MILL_NO_TAKE;
            }
        }
    }
    else {
        room->game_positions[pos2_id] = P2;
        if (is_mill(room->game_positions, pos2_id, P2) == TRUE) {
            if (isTakeStonePossible(room->game_positions, player)) {
                return TURN_OK_MILL;
            }
            else {
                return TURN_OK_MILL_NO_TAKE;
            }
        }
    }

    return TURN_OK_NOT_MILL;
}

/* Take the opponents stone from the particular game position */
response take_stone(room *room, client *player, int pos_id) {

    if (player->is_player1) {
        if (room->game_positions[pos_id] != P2) {
            return TK_STONE_ERR_MISSING_STONE; // On the position is not opponents stone
        }
        if (is_mill(room->game_positions, pos_id, P2) == TRUE) {
            return TK_STONE_ERR_IN_MILL; // The stone on the specific position is inside a mill
        }
        room->p2_stones--;
        room->game_positions[pos_id] = FREE;
        if (room->p2_stones < 3)
            return TK_STONE_OK_WIN; // Game over - opponent has less than 3 stones
    }
    else {
        if (room->game_positions[pos_id] != P1) {
            return TK_STONE_ERR_MISSING_STONE;
        }
        if (is_mill(room->game_positions, pos_id, P1) == TRUE) {
            return TK_STONE_ERR_IN_MILL;
        }
        room->p1_stones--;
        room->game_positions[pos_id] = FREE;
        if (room->p1_stones < 3)
            return TK_STONE_OK_WIN;
    }

    return TK_STONE_OK_CONT;
}
