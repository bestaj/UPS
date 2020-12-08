#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <pthread.h>
#include "client.h"
#include "message.h"
#include "room.h"

#define MAX_CLIENTS 2
#define MAX_ROOMS 32
#define BUFF_SIZE 128
#define MAX_FUNCS 20

#define CLR 0
#define ADD 1
#define UPDATE 2

#define NOW 1
#define LATER 0
#define ERROR -1
#define NO_ERROR 0

int process_login(char *params, client *player);
int process_rooms(char *params, client *player);
int process_find(char *params, client *player);
int process_create(char *params, client *player);
int process_join(char *params, client *player);
int process_logout(char *params, client *player);
int process_leave(char *params, client *player);
int process_turn(char *params, client *player);
int process_take_stone(char *params, client *player);
int process_opp_con_ok(char *params, client *player);
int process_opp_turn_ok(char *params, client *player);
int process_opp_take_stone_ok(char *params, client *player);
int process_opp_leave_ok(char *params, client *player);
int process_opp_lost_con_ok(char *params, client *player);
int process_opp_recon_ok(char *params, client *player);
int process_opp_discon_ok(char *params, client *player);
int process_update_room_ok(char *params, client *player);
int process_ping_ok(char *params, client *player);
int process_init_ok(char *params, client *player);

int send_message(int client_socket, int clear_buff);
void disconnect_client(client *client, int remove);
void inform_clients_in_lobby(int room_number, int change, char *nickname);

/* Array of pointers to functions which process particular response from the client.
  Response ID is index to the array.
*/
int (*process_func[]) (char *, client *) = {process_login, process_rooms, process_find, process_create, process_join, process_logout, process_leave, process_turn,
    process_take_stone, process_opp_con_ok, process_opp_turn_ok, process_opp_take_stone_ok, process_opp_leave_ok, process_opp_lost_con_ok, process_opp_recon_ok,
    process_opp_discon_ok, process_update_room_ok, process_ping_ok, process_init_ok};

fd_set read_fds;  // Set of filedescriptors
char recv_buff[BUFF_SIZE]; // Buffer for receiving messages from clients
char resp_buff[BUFF_SIZE]; // Buffer for responses to clients
client *clients[MAX_CLIENTS]; // Array of all connected clients
client *discon_clients[MAX_CLIENTS]; // Array of all disconnected clients waiting for possible reconnection
room *rooms[MAX_ROOMS]; // Array of rooms
struct sockaddr_in my_addr, remote_addr;
int server_socket;
int len_addr, clients_counter = 0, discon_clients_counter = 0, rooms_counter = 0;

/* Validate a nickname of client */
int test_nickname(char *nickname, client *player) {
    if (!nickname || !player) return ERROR;

    // Test if nickname contains semicolon
    if (strchr(nickname, ';')) {
        sprintf(resp_buff, get_response(LOGIN_ERR_NICK_WITH_SEMICOLON));
        send_message(player->socket, TRUE);
        return ERROR;
    }

    // Test if nickname starts/ends with whitespace
    if (isspace(nickname[0]) || isspace(nickname[strlen(nickname) - 1])) {
        sprintf(resp_buff, get_response(LOGIN_ERR_NICK_WHITESPACE));
        send_message(player->socket, TRUE);
        return ERROR;
    }

    // Test if nickname is too long
    if (strlen(nickname) > MAX_LEN_NICK) {
        sprintf(resp_buff, get_response(LOGIN_ERR_NICK_TOO_LONG));
        send_message(player->socket, TRUE);
        return ERROR;
    }
    return NO_ERROR;
}

/* Process CONNECT_OK response from the client.
   Client confirms connection into the game.
*/
int process_init_ok(char *params, client *player) {
    if (params || !player) return ERROR;

    player->state = ST_CONNECTED;
    return NO_ERROR;
}

int reconnect_client(client *new_client, client *old_client) {
    int i;
    client *opp;
    room *room;
    int *game_pos;

    strcpy(new_client->nickname, old_client->nickname);
    new_client->room_number = old_client->room_number;
    new_client->state = old_client->last_state;
    new_client->is_player1 = old_client->is_player1;
    new_client->previous_state = old_client->previous_state;
    discon_clients[old_client->id] = NULL;
    printf("discon clients: %d %p\n", old_client->id, discon_clients[old_client->id]);
    discon_clients_counter--;
    remove_client(old_client);

    if (new_client->state == ST_LOBBY) {
        printf("lobby\n");
        sprintf(resp_buff, get_response(LOGIN_EXIST_LOBBY));
        if (send_message(new_client->socket, TRUE)) return ERROR;
    }
    else { // Client was in the game
          printf("in game\n");
          if (new_client->room_number == NOT_IN_ROOM) { // If the room is not exist anymore
              sprintf(resp_buff, get_response(LOGIN_ERR_GAME_IS_OVER));
              if (send_message(new_client->socket, TRUE)) return ERROR;
              new_client->state = ST_LOBBY;
          }
          else {  // The room still exists
              room = rooms[new_client->room_number];
              if (new_client->is_player1) {
                  room->player1 = new_client;
              }
              else {
                  room->player2 = new_client;
              }
              game_pos = room->game_positions;
              opp = get_opponent(room, new_client);
              if (new_client->state == ST_WAITING_FOR_OPP) {
                  if (opp == NULL) { // Still waiting for the opponent connection
                      printf("no opponent yet\n");
                      sprintf(resp_buff, get_response(LOGIN_EXIST_WAITING_FREE), room->number);
                      if (send_message(new_client->socket, TRUE)) return ERROR;
                  }
                  else {
                      printf("some opponent: %s\n", opp->nickname);
                      if (opp->state == ST_DISCONNECTED) { // If opponent also lost connection
                          new_client->state = ST_OPP_LOST_CON;
                          sprintf(resp_buff, get_response(LOGIN_EXIST_WAITING_FULL_NOT_READY), room->number, opp->nickname);
                          if (send_message(new_client->socket, TRUE)) return ERROR;
                      }
                      else {  // Opponent is ready to play
                          new_client->state = ST_MY_TURN;
                          sprintf(resp_buff, get_response(LOGIN_EXIST_WAITING_FULL_READY), room->number, opp->nickname);
                          if (send_message(new_client->socket, TRUE)) return ERROR;
/*
                          sprintf(resp_buff, get_response(OPP_RECON));
                          if (send_message(opp->socket, TRUE)) return ERROR;
*/
                      }
                  }
              }
              else if (new_client->state == ST_OPP_LOST_CON) {
                  if (opp->state == ST_DISCONNECTED) { // Opponent is still disconnected
                      if (new_client->is_player1) {
                          sprintf(resp_buff, get_response(LOGIN_EXIST_OPP_LOST_CON_P1_NOT_READY), room->number, opp->nickname, get_state(new_client->previous_state), room->p1_unset_stones, room->p2_unset_stones,
                          game_pos[0], game_pos[1], game_pos[2], game_pos[3], game_pos[4], game_pos[5], game_pos[6], game_pos[7], game_pos[8], game_pos[9], game_pos[10], game_pos[11], game_pos[12], game_pos[13],
                          game_pos[14], game_pos[15], game_pos[16], game_pos[17], game_pos[18], game_pos[19], game_pos[20], game_pos[21], game_pos[22], game_pos[23]);
                          if (send_message(new_client->socket, TRUE)) return ERROR;
                      }
                      else {
                          sprintf(resp_buff, get_response(LOGIN_EXIST_OPP_LOST_CON_P2_NOT_READY), room->number, opp->nickname, get_state(new_client->previous_state), room->p2_unset_stones, room->p1_unset_stones,
                          game_pos[0], game_pos[1], game_pos[2], game_pos[3], game_pos[4], game_pos[5], game_pos[6], game_pos[7], game_pos[8], game_pos[9], game_pos[10], game_pos[11], game_pos[12], game_pos[13],
                          game_pos[14], game_pos[15], game_pos[16], game_pos[17], game_pos[18], game_pos[19], game_pos[20], game_pos[21], game_pos[22], game_pos[23]);
                          if (send_message(new_client->socket, TRUE)) return ERROR;
                      }

                  }
                  else {
                      if (new_client->is_player1) {
                          sprintf(resp_buff, get_response(LOGIN_EXIST_OPP_LOST_CON_P1_READY), room->number, opp->nickname, get_state(new_client->previous_state), room->p1_unset_stones, room->p2_unset_stones,
                          game_pos[0], game_pos[1], game_pos[2], game_pos[3], game_pos[4], game_pos[5], game_pos[6], game_pos[7], game_pos[8], game_pos[9], game_pos[10], game_pos[11], game_pos[12], game_pos[13],
                          game_pos[14], game_pos[15], game_pos[16], game_pos[17], game_pos[18], game_pos[19], game_pos[20], game_pos[21], game_pos[22], game_pos[23]);
                          if (send_message(new_client->socket, TRUE)) return ERROR;
                      }
                      else {
                          sprintf(resp_buff, get_response(LOGIN_EXIST_OPP_LOST_CON_P2_READY), room->number, opp->nickname, get_state(new_client->previous_state), room->p2_unset_stones, room->p1_unset_stones,
                          game_pos[0], game_pos[1], game_pos[2], game_pos[3], game_pos[4], game_pos[5], game_pos[6], game_pos[7], game_pos[8], game_pos[9], game_pos[10], game_pos[11], game_pos[12], game_pos[13],
                          game_pos[14], game_pos[15], game_pos[16], game_pos[17], game_pos[18], game_pos[19], game_pos[20], game_pos[21], game_pos[22], game_pos[23]);
                          if (send_message(new_client->socket, TRUE)) return ERROR;
                      }
                  }

              }
              else { // The client was in some of the following state: MY_TURN, OPP_TURN, TAKING_STONE, OPP_TAKING_STONE
                  if (opp->state == ST_DISCONNECTED) {
                      if (new_client->is_player1) {
                          sprintf(resp_buff, get_response(LOGIN_EXIST_ACTIVE_STATE_P1_NOT_READY), get_state(new_client->state), room->number, opp->nickname, room->p1_unset_stones, room->p2_unset_stones,
                          game_pos[0], game_pos[1], game_pos[2], game_pos[3], game_pos[4], game_pos[5], game_pos[6], game_pos[7], game_pos[8], game_pos[9], game_pos[10], game_pos[11], game_pos[12], game_pos[13],
                          game_pos[14], game_pos[15], game_pos[16], game_pos[17], game_pos[18], game_pos[19], game_pos[20], game_pos[21], game_pos[22], game_pos[23]);
                          if (send_message(new_client->socket, TRUE)) return ERROR;
                      }
                      else {
                          sprintf(resp_buff, get_response(LOGIN_EXIST_ACTIVE_STATE_P2_NOT_READY), get_state(new_client->state), room->number, opp->nickname, room->p2_unset_stones, room->p1_unset_stones,
                          game_pos[0], game_pos[1], game_pos[2], game_pos[3], game_pos[4], game_pos[5], game_pos[6], game_pos[7], game_pos[8], game_pos[9], game_pos[10], game_pos[11], game_pos[12], game_pos[13],
                          game_pos[14], game_pos[15], game_pos[16], game_pos[17], game_pos[18], game_pos[19], game_pos[20], game_pos[21], game_pos[22], game_pos[23]);
                          if (send_message(new_client->socket, TRUE)) return ERROR;
                      }
                  }
                  else {
                      if (new_client->is_player1) {
                          sprintf(resp_buff, get_response(LOGIN_EXIST_ACTIVE_STATE_P1_READY), get_state(new_client->state), room->number, opp->nickname, room->p1_unset_stones, room->p2_unset_stones,
                          game_pos[0], game_pos[1], game_pos[2], game_pos[3], game_pos[4], game_pos[5], game_pos[6], game_pos[7], game_pos[8], game_pos[9], game_pos[10], game_pos[11], game_pos[12], game_pos[13],
                          game_pos[14], game_pos[15], game_pos[16], game_pos[17], game_pos[18], game_pos[19], game_pos[20], game_pos[21], game_pos[22], game_pos[23]);
                          if (send_message(new_client->socket, TRUE)) return ERROR;
                      }
                      else {
                          sprintf(resp_buff, get_response(LOGIN_EXIST_ACTIVE_STATE_P2_READY), get_state(new_client->state), room->number, opp->nickname, room->p2_unset_stones, room->p1_unset_stones,
                          game_pos[0], game_pos[1], game_pos[2], game_pos[3], game_pos[4], game_pos[5], game_pos[6], game_pos[7], game_pos[8], game_pos[9], game_pos[10], game_pos[11], game_pos[12], game_pos[13],
                          game_pos[14], game_pos[15], game_pos[16], game_pos[17], game_pos[18], game_pos[19], game_pos[20], game_pos[21], game_pos[22], game_pos[23]);
                          if (send_message(new_client->socket, TRUE)) return ERROR;
                      }
                  }
              }

              if (opp != NULL) {
                  // Inform the opponent about the reconnection of his opponent
                  sprintf(resp_buff, get_response(OPP_RECON));
                  if (send_message(opp->socket, TRUE)) return ERROR;
              }
          }
    }

    return NO_ERROR;
}

/* Process LOGIN request from the client.
  Client wants to log in with a specific nickname.
*/
int process_login(char *params, client *player) {
    char *login_type, *nickname;
    int i, found = FALSE;

    if (!params || !player) return ERROR;

    // Get 1.parameter - NEW | EXIST and 2. parameter - nickname
    login_type = strtok_r(params, SEMICOLON, &nickname);
    if (!login_type || !nickname || (strlen(nickname) == 0)) return ERROR;

    // Log in a NEW client
    if (strncmp(login_type, "NEW", strlen(login_type)) == 0) {
        if (test_nickname(nickname, player)) return NO_ERROR; // Validate nickname

        // Test if nickname already exists
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (i == player->id) continue; // Skip the same client

            if (clients[i]) { // If there is some client on that position in array, then compare nicknames
                if (strcmp(clients[i]->nickname, nickname) == 0) {
                    sprintf(resp_buff, get_response(LOGIN_ERR_NICK_EXIST), nickname);
                    if (send_message(player->socket, TRUE)) return ERROR;
                    return NO_ERROR;
                }
            }
            if (discon_clients[i]) { // Go even through temporarily disconnected clients
                if (strcmp(discon_clients[i]->nickname, nickname) == 0) {
                    sprintf(resp_buff, get_response(LOGIN_ERR_NICK_EXIST), nickname);
                    if (send_message(player->socket, TRUE)) return ERROR;
                    return NO_ERROR;
                }
            }
        }

        // Nickname is OK - save nickname for the client
        strncpy(player->nickname, nickname, strlen(nickname));
        sprintf(resp_buff, get_response(LOGIN_NEW_OK));
        if (send_message(player->socket, TRUE)) return ERROR;
        player->state = ST_LOBBY;
    }
    // Log in an EXISTING client
    else if (strncmp(login_type, "EXIST", strlen(login_type)) == 0) {
        if (test_nickname(nickname, player)) return NO_ERROR;

        for (i = 0; i < MAX_CLIENTS; i++) {
            if (!discon_clients[i]) continue;
            printf("someone %d nickname:%s\n", i, discon_clients[i]->nickname);

            if (strcmp(nickname, discon_clients[i]->nickname) == 0) { // Compare nicknames
                printf("nicknames");
                if (strcmp(player->address, discon_clients[i]->address) == 0) { // Compare addresses
                    printf("found client\n");
                    pthread_cancel(discon_clients[i]->thread_id);
                    printf("before reconnect\n");
                    if (reconnect_client(player, discon_clients[i]) == ERROR) return ERROR;
                    return NO_ERROR;
                }
            }
        }
        printf("no disc clients\n");
        sprintf(resp_buff, get_response(LOGIN_ERR_CLIENT_NOT_FOUND));
        if (send_message(player->socket, TRUE)) return ERROR;
        disconnect_client(player, NOW);
    }
    // Invalid 1. parameter (NEW or EXIST)
    else return ERROR;

    return NO_ERROR;
}

/* Process ROOMS request from the client.
  Client wants to get a list of all rooms.
*/
int process_rooms(char *params, client *player) {
    int i;

    if (params || !player) return ERROR; // message ROOMS has no params

    if (rooms_counter == 0) { // If there is no created rooms
        sprintf(resp_buff, get_response(ROOMS_ERR));
        if (send_message(player->socket, TRUE)) return ERROR;
    }
    else {
        for (i = 0; i < MAX_ROOMS; i++) { // Go through all rooms and send to client info about every single room
            if (!rooms[i]) continue;

            if (rooms[i]->room_state == FREE) {
                sprintf(resp_buff, get_response(ROOMS_OK_FREE), rooms[i]->number, rooms[i]->player1->nickname);
                if (send_message(player->socket, TRUE)) return ERROR;
            }
            else {
                sprintf(resp_buff, get_response(ROOMS_OK_FULL), rooms[i]->number, rooms[i]->player1->nickname, rooms[i]->player2->nickname);
                if (send_message(player->socket, TRUE)) return ERROR;
            }
        }
    }

    return NO_ERROR;
}

/* Add the client into found free room */
int join_into_found_room(room *room, client *player) {
    if (!room || !player) return ERROR;

    // Set the room and client
    room->player2 = player;
    room->room_state = FULL;
    player->room_number = room->number;
    player->is_player1 = FALSE;

    if (room->player1->state == ST_DISCONNECTED) { // If opponent lost connection
        player->previous_state = ST_OPP_TURN;
        player->state = ST_OPP_LOST_CON;
        sprintf(resp_buff, get_response(FIND_OK_OPP_LOST_CON), room->number, room->player1->nickname);
        if (send_message(player->socket, TRUE)) return ERROR;
    }
    else {
        player->state = ST_OPP_TURN;
        sprintf(resp_buff, get_response(FIND_OK_OPP_READY), room->number, room->player1->nickname);
        if (send_message(player->socket, TRUE)) return ERROR;

        sprintf(resp_buff, get_response(OPP_CON), player->nickname);
        if (send_message(room->player1->socket, TRUE)) return ERROR;
    }

    inform_clients_in_lobby(room->number, UPDATE, player->nickname);
    return NO_ERROR;
}

/* Process FIND request from the client.
  Client wants to join into any free room.
*/
int process_find(char *params, client *player) {
    int i;

    if (params || !player) return ERROR; // message FIND has no params

    for (i = 0; i < MAX_ROOMS; i++) { // Go through all rooms and finding some free room
        if (!rooms[i]) continue;

        if (rooms[i]->room_state == FREE) {
            printf("room: %s\n", rooms[i]->player1->nickname);
            if (join_into_found_room(rooms[i], player)) return ERROR; // Add the client into the found room
            return NO_ERROR;
        }
    }

    sprintf(resp_buff, get_response(FIND_ERR));
    if (send_message(player->socket, TRUE)) return ERROR;

    return NO_ERROR;
}

/* Process CREATE request from the client.
  Client wants to create a new room.
*/
int process_create(char *params, client *player) {
    int i;

    if (params || !player) return ERROR; // message CREATE has no params

    if (rooms_counter == MAX_ROOMS) { // If maximum rooms was reached
        sprintf(resp_buff, get_response(CREATE_ERR));
        if (send_message(player->socket, TRUE)) return ERROR;
        return NO_ERROR;
    }

    for (i = 0; i < MAX_ROOMS; i++) { // Finding a free place in array to create a room
        if (rooms[i] == NULL) {
            rooms[i] = create_room(i, player);
            player->room_number = i;
            player->is_player1 = TRUE;
            rooms_counter++;
            sprintf(resp_buff, get_response(CREATE_OK), i);
            if (send_message(player->socket, TRUE)) return ERROR;
            break;
        }
    }
    player->state = ST_WAITING_FOR_OPP;
    inform_clients_in_lobby(player->room_number, ADD, player->nickname);
    return NO_ERROR;
}

/* Process JOIN request from the client.
  Client wants to join into the specific room.
*/
int process_join(char *params, client *player) {
    int number; // Number of the room to join
    char *par, *rest;

    if (!params || !player) return ERROR;

    par = strtok(params, END_OF_LINE);
    if (!par || (strlen(par) == 0)) return ERROR;

    number = strtol(par, &rest, 10);
    if (*rest != '\0') return ERROR;

    // Test if room number is invalid
    if (number < 0 || number > (MAX_ROOMS - 1)) {
        sprintf(resp_buff, get_response(JOIN_ERR_INV_NUM));
        if (send_message(player->socket, TRUE)) return ERROR;
        return NO_ERROR;
    }

    // Test if the specific room exists
    if (rooms[number] == NULL) {
        sprintf(resp_buff, get_response(JOIN_ERR_INV_ROOM));
        if (send_message(player->socket, TRUE)) return ERROR;
        return NO_ERROR;
    }

    // Test if the specific room is full
    if (rooms[number]->room_state == FULL) {
        sprintf(resp_buff, get_response(JOIN_ERR_FULL_ROOM), number);
        if (send_message(player->socket, TRUE)) return ERROR;
        return NO_ERROR;
    }

    // Join the client into the room
    rooms[number]->player2 = player;
    rooms[number]->room_state = FULL;
    player->room_number = number;
    player->is_player1 = FALSE;

    if (rooms[number]->player1->state == ST_DISCONNECTED) {
        player->previous_state = ST_OPP_TURN;
        player->state = ST_OPP_LOST_CON;
        sprintf(resp_buff, get_response(JOIN_OK_OPP_LOST_CON), number, rooms[number]->player1->nickname);
        if (send_message(player->socket, TRUE)) return ERROR;
    }
    else {
        player->state = ST_OPP_TURN;
        sprintf(resp_buff, get_response(JOIN_OK_OPP_READY), number, rooms[number]->player1->nickname);
        if (send_message(player->socket, TRUE)) return ERROR;

        sprintf(resp_buff, get_response(OPP_CON), player->nickname);
        if (send_message(rooms[number]->player1->socket, TRUE)) return ERROR;
    }

    inform_clients_in_lobby(number, UPDATE, player->nickname);
    return NO_ERROR;
}

/* Process LOGOUT request from the client.
  Client wants to log out from the server.
*/
int process_logout(char *params, client *player) {
    if (params || !player) return ERROR; // message LOGOUT has no params

    sprintf(resp_buff, get_response(LOGOUT_OK), player->nickname);
    if (send_message(player->socket, TRUE)) return ERROR;
    disconnect_client(player, NOW);

    return NO_ERROR;
}

/* Send to all clients which are in lobby info that some room was changed */
void inform_clients_in_lobby(int room_number, int change, char *nickname) {
    int i;

    switch(change) {
        case CLR:
            sprintf(resp_buff, get_response(UPD_ROOM_CLR), room_number);
            break;
        case ADD:
            sprintf(resp_buff, get_response(UPD_ROOM_ADD), room_number, nickname);
            break;
        case UPDATE:
            sprintf(resp_buff, get_response(UPD_ROOM_UPDATE), room_number, nickname);
            break;
    }

    for (i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i]) continue;
        if (clients[i]->state == ST_LOBBY) {
            send_message(clients[i]->socket, FALSE);
        }
    }
}

/* Leave the room and remove it */
void exit_room(client *player) {
    int number = player->room_number;
    player->room_number = NOT_IN_ROOM;
    client *opp = get_opponent(rooms[number], player);
    if (opp) opp->room_number = NOT_IN_ROOM;
    remove_room(rooms[number]);
    rooms[number] = NULL;
    rooms_counter--;
    inform_clients_in_lobby(number, CLR, NULL);
}

/* Process LEAVE request from the client.
  Client wants to leave the room.
*/
int process_leave(char *params, client *player) {
    int number;
    client *opp;

    if (params || !player) return ERROR; // message LEAVE has no params

    number = player->room_number;
    // Test if the client is in a state WAITING_FOR_OPP, then dont have to inform the opponent about leaving
    if (player->state != ST_WAITING_FOR_OPP) {
        opp = get_opponent(rooms[number], player);
        if (opp->state != ST_DISCONNECTED) {
            sprintf(resp_buff, get_response(OPP_LEAVE));
            send_message(get_opponent(rooms[number], player)->socket, TRUE);
        }
/*        else {
            opp->room_number = NOT_IN_ROOM;
        }
*/

    }
    exit_room(player);

    sprintf(resp_buff, get_response(LEAVE_OK));
    if (send_message(player->socket, TRUE)) return ERROR;
    player->state = ST_LOBBY;
//    player->room_number = NOT_IN_ROOM;
    return NO_ERROR;
}

/* Process TURN request from the client.
  Client wants to set or shift the stone.
*/
int process_turn(char *params, client *player) {
    char *part, *rest, *rest2;
    client *opp;
    int pos1, pos2;
    response ret;

    if (!params || !player) return ERROR;

    part = strtok_r(params, SEMICOLON, &rest);
    if (!part) return ERROR;

    if (strcmp(part, "SET") == 0) { // If client set the stone
        part = strtok(rest, END_OF_LINE);
        if (!part) return ERROR;

        pos1 = strtol(part, &rest, 10);
        if (*rest != '\0') return ERROR;

        // Invalid position ID
        if (pos1 < 0 || pos1 > 23) {
            sprintf(resp_buff, get_response(TURN_ERR_INV_POS1));
            if (send_message(player->socket, TRUE)) return ERROR;
            return NO_ERROR;
        }
        ret = set_stone(rooms[player->room_number], player, pos1); // Set the stone
        // Evaluate the return value of setting the stone
        switch (ret) {
            case TURN_OK_MILL:
                sprintf(resp_buff, get_response(TURN_OK_MILL));
                if (send_message(player->socket, TRUE)) return ERROR;

                sprintf(resp_buff, get_response(OPP_TURN_SET_MILL), pos1);
                opp = get_opponent(rooms[player->room_number], player);
                if (send_message(opp->socket, TRUE)) return ERROR;
                player->state = ST_TAKING_STONE;
                break;
            case TURN_OK_MILL_TAKE_ANY_STONE:
                sprintf(resp_buff, get_response(TURN_OK_MILL_TAKE_ANY_STONE));
                if (send_message(player->socket, TRUE)) return ERROR;

                sprintf(resp_buff, get_response(OPP_TURN_SET_MILL), pos1);
                opp = get_opponent(rooms[player->room_number], player);
                if (send_message(opp->socket, TRUE)) return ERROR;
                player->state = ST_TAKING_STONE;
                break;
            case TURN_OK_NOT_MILL_WIN:
                sprintf(resp_buff, get_response(TURN_OK_NOT_MILL_WIN));
                if (send_message(player->socket, TRUE)) return ERROR;

                sprintf(resp_buff, get_response(OPP_TURN_SET_NOT_MILL_LOOSE), pos1);
                opp = get_opponent(rooms[player->room_number], player);
                if (send_message(opp->socket, TRUE)) return ERROR;
                exit_room(player);
                player->state = ST_LOBBY;
        //        player->room_number = NOT_IN_ROOM
                break;
            case TURN_OK_NOT_MILL_CONT:
                sprintf(resp_buff, get_response(TURN_OK_NOT_MILL_CONT));
                if (send_message(player->socket, TRUE)) return ERROR;

                sprintf(resp_buff, get_response(OPP_TURN_SET_NOT_MILL_CONT), pos1);
                opp = get_opponent(rooms[player->room_number], player);
                if (send_message(opp->socket, TRUE)) return ERROR;
                player->state = ST_OPP_TURN;
                break;
            case TURN_ERR_CANNOT_SET:
                sprintf(resp_buff, get_response(TURN_ERR_CANNOT_SET));
                if (send_message(player->socket, TRUE)) return ERROR;
                break;
            case TURN_ERR_FULL_POS:
                sprintf(resp_buff, get_response(TURN_ERR_FULL_POS), pos1);
                if (send_message(player->socket, TRUE)) return ERROR;
                break;
        }
    }
    else if (strcmp(part, "SHIFT") == 0) { // If client shift the stone
        part = strtok(rest, SEMICOLON);
        if (!part) return ERROR;
        pos1 = strtol(part, &rest2, 10);
        if (*rest2 != '\0') return ERROR;

        if (pos1 < 0 || pos1 > 23) {
            sprintf(resp_buff, get_response(TURN_ERR_INV_POS1));
            if (send_message(player->socket, TRUE)) return ERROR;
            return NO_ERROR;
        }

        part = strtok(NULL, END_OF_LINE);
        if (!part) return ERROR;

        pos2 = strtol(part, &rest2, 10);
        if (*rest2 != '\0') return ERROR;

        if (pos2 < 0 || pos2 > 23) {
            sprintf(resp_buff, get_response(TURN_ERR_INV_POS2));
            if (send_message(player->socket, TRUE)) return ERROR;
            return NO_ERROR;
        }

        ret = shift_stone(rooms[player->room_number], player, pos1, pos2); // Shift the stone
        // Evaluate the return value of shifting the stone
        switch (ret) {
          case TURN_OK_MILL:
              sprintf(resp_buff, get_response(TURN_OK_MILL));
              if (send_message(player->socket, TRUE)) return ERROR;

              sprintf(resp_buff, get_response(OPP_TURN_SHIFT_MILL), pos1, pos2);
              opp = get_opponent(rooms[player->room_number], player);
              if (send_message(opp->socket, TRUE)) return ERROR;
              player->state = ST_TAKING_STONE;
              break;
          case TURN_OK_MILL_TAKE_ANY_STONE:
              sprintf(resp_buff, get_response(TURN_OK_MILL_TAKE_ANY_STONE));
              if (send_message(player->socket, TRUE)) return ERROR;

              sprintf(resp_buff, get_response(OPP_TURN_SHIFT_MILL), pos1, pos2);
              opp = get_opponent(rooms[player->room_number], player);
              if (send_message(opp->socket, TRUE)) return ERROR;
              player->state = ST_TAKING_STONE;
              break;
          case TURN_OK_NOT_MILL_WIN:
              sprintf(resp_buff, get_response(TURN_OK_NOT_MILL_WIN));
              if (send_message(player->socket, TRUE)) return ERROR;

              sprintf(resp_buff, get_response(OPP_TURN_SHIFT_NOT_MILL_LOOSE), pos1, pos2);
              opp = get_opponent(rooms[player->room_number], player);
              if (send_message(opp->socket, TRUE)) return ERROR;
              exit_room(player);
              player->state = ST_LOBBY;
    //          player->room_number = NOT_IN_ROOM;
              break;
          case TURN_OK_NOT_MILL_CONT:
              sprintf(resp_buff, get_response(TURN_OK_NOT_MILL_CONT));
              if (send_message(player->socket, TRUE)) return ERROR;

              sprintf(resp_buff, get_response(OPP_TURN_SHIFT_NOT_MILL_CONT), pos1, pos2);
              opp = get_opponent(rooms[player->room_number], player);
              if (send_message(opp->socket, TRUE)) return ERROR;
              player->state = ST_OPP_TURN;
              break;
          case TURN_ERR_CANNOT_SHIFT:
              sprintf(resp_buff, get_response(TURN_ERR_CANNOT_SHIFT));
              if (send_message(player->socket, TRUE)) return ERROR;
              break;
          case TURN_ERR_MISSING_STONE:
              sprintf(resp_buff, get_response(TURN_ERR_MISSING_STONE));
              if (send_message(player->socket, TRUE)) return ERROR;
              break;
          case TURN_ERR_WRONG_POS2:
              sprintf(resp_buff, get_response(TURN_ERR_WRONG_POS2));
              if (send_message(player->socket, TRUE)) return ERROR;
              break;
        }
    }
    else return ERROR;

    return NO_ERROR;
}

/* Process TAKE_STONE request from the client.
  Client wants to take opponents stone.
*/
int process_take_stone(char *params, client *player) {
    char *rest;
    int pos, ret;
    client *opp;

    if (!params || !player) return ERROR;

    pos = strtol(params, &rest, 10);
    if (*rest != '\0') return ERROR;

    if (pos < 0 || pos > 23) {
        sprintf(resp_buff, get_response(TK_STONE_ERR_INV_POS));
        if (send_message(player->socket, TRUE)) return ERROR;
        return NO_ERROR;
    }

    ret = take_stone(rooms[player->room_number], player, pos); // Take stone
    // Evaluate the return value of taking the stone
    switch (ret) {
        case TK_STONE_OK_CONT:
            sprintf(resp_buff, get_response(TK_STONE_OK_CONT));
            if (send_message(player->socket, TRUE)) return ERROR;

            sprintf(resp_buff, get_response(OPP_TK_STONE_CONT), pos);
            opp = get_opponent(rooms[player->room_number], player);
            if (send_message(opp->socket, TRUE)) return ERROR;
            player->state = ST_OPP_TURN;
            break;
        case TK_STONE_OK_WIN:
            sprintf(resp_buff, get_response(TK_STONE_OK_WIN));
            if (send_message(player->socket, TRUE)) return ERROR;

            sprintf(resp_buff, get_response(OPP_TK_STONE_LOOSE), pos);
            opp = get_opponent(rooms[player->room_number], player);
            if (send_message(opp->socket, TRUE)) return ERROR;
            exit_room(player);
            player->state = ST_LOBBY;
      //      player->room_number = NOT_IN_ROOM;
            break;
        case TK_STONE_ERR_MISSING_STONE:
            sprintf(resp_buff, get_response(TK_STONE_ERR_MISSING_STONE));
            if (send_message(player->socket, TRUE)) return ERROR;
            break;
        case TK_STONE_ERR_IN_MILL:
            sprintf(resp_buff, get_response(TK_STONE_ERR_IN_MILL));
            if (send_message(player->socket, TRUE)) return ERROR;
            break;
    }

    return NO_ERROR;
}

/* Process client response that his opponent was connected into the game. */
int process_opp_con_ok(char *params, client *player) {
    if (params || !player) return ERROR;
    player->state = ST_MY_TURN;

    return NO_ERROR;
}

/* Process client response that his opponent turned */
int process_opp_turn_ok(char *params, client *player) {
    char *par, *rest;
    if (!params || !player) return ERROR;

    if (strcmp(params, "MILL") == 0) {
        player->state = ST_OPP_TAKING_STONE;
    }
    else {
        par = strtok_r(params, SEMICOLON, &rest);
        if (!par) return ERROR;

        if (strcmp(par, "NOT_MILL") == 0) {
            if (strcmp(rest, "LOOSE") == 0) {
                player->state = ST_LOBBY;
      //          player->room_number = NOT_IN_ROOM;
            }
            else if (strcmp(rest, "CONTINUE") == 0) {
                player->state = ST_MY_TURN;
            }
            else {
                return ERROR;
            }
        }
        else {
            return ERROR;
        }
    }

    return NO_ERROR;
}

/* Process client response that his opponent took a stone */
int process_opp_take_stone_ok(char *params, client *player) {
    if (!params || !player) return ERROR;

    if (strcmp(params, "CONTINUE") == 0) {
        player->state = ST_MY_TURN;
    }
    else if (strcmp(params, "LOOSE") == 0) {
        player->state = ST_LOBBY;
//        player->room_number = NOT_IN_ROOM;
    }
    else
        return ERROR;

    return NO_ERROR;
}

/* Process client response that his opponent leaved the room */
int process_opp_leave_ok(char *params, client *player) {
    if (params || !player) return ERROR;
    player->state = ST_LOBBY;
//    player->room_number = NOT_IN_ROOM;

    return NO_ERROR;
}

/* Process client response that his opponent lost connection */
int process_opp_lost_con_ok(char *params, client *player) {
    if (params || !player) return ERROR;
    player->previous_state = player->state;
    player->state = ST_OPP_LOST_CON;

    return NO_ERROR;
}

int process_opp_recon_ok(char *params, client *player) {
    if (params || !player) return ERROR;
    player->state = player->previous_state;
    return NO_ERROR;
}


int process_opp_discon_ok(char *params, client *player) {
    if (params || !player) return ERROR;
    exit_room(player);
//    player->room_number = NOT_IN_ROOM;
    player->state = ST_LOBBY;
    return NO_ERROR;
}


int process_update_room_ok(char *params, client *player) {
    if (params || !player) return ERROR;
    return NO_ERROR;
}


int process_ping_ok(char *params, client *player) {
    return NO_ERROR;
}


/*  Thread function for waiting 30s for possible client reconnection.
    If client will not recconect then remove client the array of disconnected clients.
    par arg... disconnected clients
*/
void *start_countdown(void *arg) {
    client *c = (client *)arg;
    client *opp;
    printf("thread is running...\n");
    sleep(40);

    printf("thread is over\n");
    // After 30s remove the client
    if (c->room_number != NOT_IN_ROOM) { // If client was in the game then inform the opponent about disconnection of his opponent
        opp = get_opponent(rooms[c->room_number], c);
        if ((opp != NULL) && (opp->state != ST_DISCONNECTED)) {
            sprintf(resp_buff, get_response(OPP_DISCON));
            send_message(opp->socket, TRUE);
        }
    }
    discon_clients[c->id] = NULL;
    remove_client(c);
    discon_clients_counter--;
}


/*  Store the client state for his possible reconnection during next 30s.
    par client... disconnected clients
    return 0 = OK, -1 = ERROR
*/
int store_disconnected_client(client *player) {
    int i;

    if (player->room_number != NOT_IN_ROOM) { // If client is in the game
        if ((player->state != ST_WAITING_FOR_OPP) && (player->state != ST_OPP_LOST_CON)) {  // If client is not waiting for the opponent re/connection, then inform the opponent about disconnection
            sprintf(resp_buff, get_response(OPP_LOST_CON));
            if (send_message(get_opponent(rooms[player->room_number], player)->socket, TRUE)) return ERROR;
        }
    }

    player->last_state = player->state;
    player->state = ST_DISCONNECTED;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!discon_clients[i]) {
            player->id = i;
            printf("discon id: %d\n", i);
            discon_clients[i] = player;
            discon_clients_counter++;
            break;
        }
    }

    // Start thread which counting 30s for reconnection
    if (pthread_create(&(player->thread_id), NULL, (void *)&start_countdown, (void *)player)) {
        return ERROR;
    }

    return NO_ERROR;
}

/* Disconnect the client or put him into the array of disconnected clients for possible reconnection.
   par remove LATER - store the client for possible reconnection, NOW - remove client immedietly
*/
void disconnect_client(client *player, int remove) {
    int i;
    client *opp;

    // Get info about disconnected client
    getpeername(player->socket, (struct sockaddr *) &remote_addr, &len_addr);
    printf("Client %s %d was disconnected.\n", inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port));
    close(player->socket);
    clients[player->id] = NULL;
    clients_counter--;
    if (remove == LATER) {
        store_disconnected_client(player);
    }
    else { // remove client
        if (player->room_number != NOT_IN_ROOM) { // If client was in the game then inform the opponent about disconnection of his opponent
            opp = get_opponent(rooms[player->room_number], player);
            if ((opp != NULL) && (opp->state != ST_DISCONNECTED)) {
                sprintf(resp_buff, get_response(OPP_DISCON));
                send_message(opp->socket, TRUE);
            }
            else {
                exit_room(player);
            }
        }
        remove_client(player);
    }
}

/*  Test if it is a new client.
    Compare the address with all addresses of all disconnected clients.
    par address... address of the connected client
    return 1 = new client, 0 = disconnected client
*/
int is_new_client(char *address) {
    int i;

    for (i = 0; i < MAX_CLIENTS; i++) {
        if (!discon_clients[i]) continue;

        if (strcmp(discon_clients[i]->address, address) == 0) {
            printf("old client\n");
            return FALSE;
        }
    }
    printf("new client\n");
    return TRUE;
}

/* Send a message to the client.
   par client_socket... socket of the client, to who we are sending a message
   par clear_buff... says if buffer is cleared after send
   return 0 = OK, -1 = ERROR
*/
int send_message(int client_socket, int clear_buff) {
    if (client_socket < 0) return ERROR;
    if (send(client_socket, resp_buff, strlen(resp_buff), 0) <= 0) {
        printf("Sending failed.\n");
        return ERROR;
    }
    printf("Send to(%d): %s", client_socket, resp_buff);
    if (clear_buff == TRUE) {
        memset(resp_buff, 0, sizeof(resp_buff));
    }
    return NO_ERROR;
}

/* Receive a message from the client and store it into the buffer.
   par client... client who sent a message
   return 0 = OK, -1 = ERROR
*/
int read_message(client *client) {
    message *msg;
    int ret_value;

    if (!client) return ERROR;

    memset(recv_buff, 0, sizeof(recv_buff)); // Clear the buffer
    if (recv(client->socket, recv_buff, sizeof(recv_buff), 0) <= 0) {
        return ERROR;
    }
    printf("Received from(%d): %s", client->socket, recv_buff);
    msg = parse_msg(recv_buff);
    if (!msg) return ERROR;

//    printf("msg_id: %d\nmsg_name: %s\ndata: %s\n", msg->id, msg->name, msg->params);

    // If received message is allowed in the current client state
    if (verify_transition(client->state, msg->id) == ST_INVALID) {
         printf("Invalid event %s(%d) in the current state %s.\n", msg->name, msg->id, get_state(client->state));
        return ERROR;
    }

    // Process message from the client
    if (process_func[msg->id](msg->params, client) == ERROR) {
        printf("Processing message failed.\n");
        return ERROR;
    }
    free_message(&msg);

    return NO_ERROR;
}

 /* Create a server and receive messages from the clients.
    par port... port on which server is listening
    return 0 = OK, -1 = ERROR
 */
int create_server(int port) {
    int i, sd, max_sd, bytes, par = 1;
    int client_socket;

    // Create a server socket and set the options
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char *) &par, sizeof(int))) {
        printf("Problems with server socket settings.\n");
        return ERROR;
    }

    // Set the address of the server socket
    memset(&my_addr, 0, sizeof(struct sockaddr_in));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind server socket with the address
    if (bind(server_socket, (struct sockaddr *) &my_addr, sizeof(struct sockaddr_in))) {
        printf("Binding failed.\n");
        return ERROR;
    }

    // Waiting for incoming connections
    if (listen(server_socket, 5)) {
        printf("Listening failed.\n");
        return ERROR;
    }

    printf("Server is running...\n");
    len_addr = sizeof(remote_addr);

    while (1) {
        FD_ZERO(&read_fds); // File descriptor set is filled by 0
        FD_SET(server_socket, &read_fds); // Add server socket into the set
        max_sd = server_socket;

        for (i = 0; i < MAX_CLIENTS; i++) { // Add all client sockets into the set
            if (!clients[i]) continue;

            sd = clients[i]->socket;
            if (sd > 0)
                FD_SET(sd, &read_fds);
            if (sd > max_sd)
                max_sd = sd;
        }

        // Waiting for some activity on some socket
        if (select(max_sd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            printf("Select failed.\n");
            return ERROR;
        }

        if (FD_ISSET(server_socket, &read_fds)) {
            // Activity on the server socket - connecting a new client
            client_socket = accept(server_socket, (struct sockaddr *) &remote_addr, &len_addr);
            if (client_socket < 0) {
                printf("Accepting client failed.\n");
                return ERROR;
            }
            if (((clients_counter + discon_clients_counter) == MAX_CLIENTS) && is_new_client(inet_ntoa(remote_addr.sin_addr))) {

                sprintf(resp_buff, get_response(INIT_ERR));
                if (send_message(client_socket, TRUE)) return ERROR;
                printf("Maximum clients was reached. Client %s was refused.\n", inet_ntoa(remote_addr.sin_addr));
                close(client_socket);
            }
            else {
                printf("New client %s:%d (socket: %d) is connected.\n", inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port), client_socket);

                for (i = 0; i < MAX_CLIENTS; i++) {
                    if (!clients[i]) {
                        clients[i] = create_client(inet_ntoa(remote_addr.sin_addr), client_socket, i);
                        clients_counter++;
                        break;
                    }
                }
                sprintf(resp_buff, get_response(INIT_OK));
                if (send_message(client_socket, TRUE)) return ERROR;
            }
        }

        for (i = 0; i < MAX_CLIENTS; i++) {
            if (!clients[i]) continue;
            sd = clients[i]->socket;

            if (FD_ISSET(sd, &read_fds)) {
                // Count of bytes ready to read
                ioctl(sd, FIONREAD, &bytes);
                if (bytes > 0) {
                    // Read message from the client
                    if (read_message(clients[i]) == ERROR) {
                        sprintf(resp_buff, get_response(UNKNOWN_MSG));
                        send_message(clients[i]->socket, TRUE);
                        disconnect_client(clients[i], NOW);
                    }
                }
                else {  // Problems with the client socket - store the client state for a possible future reconnection, if he is not in state INIT or CONNECTED
                    if (clients[i]->state == ST_INIT || clients[i]->state == ST_CONNECTED) {
                        disconnect_client(clients[i], NOW);
                    }
                    else {
                        disconnect_client(clients[i], LATER);
                    }
                }
            }
        }
    }
    close(server_socket);
    return NO_ERROR;
}

/* The main function of the program
   par argc... count of arguments
   par argv... array of command line arguments, argv[1] = port number
*/
int main(int argc, char *argv[]) {
    int port;
    char *rest;

    if (argc < 2) {
        printf("Missing argument.\nUsage: server [port]\nport: <0 - 65536>\n");
        return EXIT_FAILURE;
    }

    port = strtol(argv[1], &rest, 10);
    if (*rest != '\0' || port < 0 || port > 65536) {  // Valid port number
        printf("Invalid port.\nport: <0 - 65536>\n");
        return EXIT_FAILURE;
    }

    if (create_server(port) == ERROR) {
        printf("Some problems occured.\n");
        close(server_socket);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
