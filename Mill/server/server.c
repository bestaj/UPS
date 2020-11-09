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
#include "client.h"
#include "message.h"
#include "room.h"

#define MAX_CLIENTS 64
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

int send_message(int client_socket, int clear_buff);
void disconnect_client(client *client, int remove);
void inform_clients_in_lobby(int room_number, int change, char *nickname);

int (*process_func[]) (char *, client *) = {process_login, process_rooms, process_find, process_create, process_join, process_logout, process_leave, process_turn,
    process_take_stone, process_opp_con_ok, process_opp_turn_ok, process_opp_take_stone_ok, process_opp_leave_ok, process_opp_lost_con_ok, process_opp_recon_ok,
    process_opp_discon_ok, process_update_room_ok, process_ping_ok};

fd_set read_fds;
char recv_buff[BUFF_SIZE];
char resp_buff[BUFF_SIZE];
client *clients[MAX_CLIENTS]; // Array of all connected clients
client *discon_clients[MAX_CLIENTS];
room *rooms[MAX_ROOMS];
struct sockaddr_in my_addr, remote_addr;
int len_addr, clients_counter = 0, discon_clients_counter = 0, rooms_counter = 0;

int test_nickname(char *nickname, client *player) {

    // test if nickname contains semicolon
    if (strchr(nickname, ';')) {
        sprintf(resp_buff, get_response(LOGIN_ERR_NICK_WITH_SEMICOLON));
        send_message(player->socket, TRUE);
        return ERROR;
    }

    // test if nickname starts/ends with whitespace
    if (isspace(nickname[0]) || isspace(nickname[strlen(nickname) - 1])) {
        sprintf(resp_buff, get_response(LOGIN_ERR_NICK_WHITESPACE));
        send_message(player->socket, TRUE);
        return ERROR;
    }

    // test if nickname is too long
    if (strlen(nickname) > MAX_LEN_NICK) {
      sprintf(resp_buff, get_response(LOGIN_ERR_NICK_TOO_LONG));
      send_message(player->socket, TRUE);
      return ERROR;
    }
    return NO_ERROR;
}

int process_login(char *params, client *player) {
    char *login_type, *nickname;
    int i;

    printf("Processing LOGIN\n");
    if (!params || !player) return ERROR;

    // Get 1.parameter - NEW | EXIST and 2. parameter - nickname
    login_type = strtok_r(params, SEMICOLON, &nickname);
    if (!login_type || !nickname || (strlen(nickname) == 0)) return ERROR;

    // Log in a NEW client
    if (strncmp(login_type, "NEW", strlen(login_type)) == 0) {
        if (test_nickname(nickname, player)) return NO_ERROR;

        // test if nickname already exists
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (i == player->id) continue;

            if (clients[i]) {
                if (strcmp(clients[i]->nickname, nickname) == 0) {
                    sprintf(resp_buff, get_response(LOGIN_ERR_NICK_EXIST), nickname);
                    if (send_message(player->socket, TRUE)) return ERROR;
                    return NO_ERROR;
                }
            }
            if (discon_clients[i]) {
                if (strcmp(discon_clients[i]->nickname, nickname) == 0) {
                    sprintf(resp_buff, get_response(LOGIN_ERR_NICK_EXIST), nickname);
                    if (send_message(player->socket, TRUE)) return ERROR;
                    return NO_ERROR;
                }
            }
        }

        // nickname is OK - save nickname to the client
        strncpy(player->nickname, nickname, strlen(nickname));
        sprintf(resp_buff, get_response(LOGIN_NEW_OK));
        if (send_message(player->socket, TRUE)) return ERROR;
        player->state = ST_LOBBY;
    }
    // Log in an EXISTING client
    else if (strncmp(login_type, "EXIST", strlen(login_type)) == 0) {
        if (test_nickname(nickname, player)) return NO_ERROR;

          // doplnit

    }
    // Invalid parameter
    else return ERROR;

    return NO_ERROR;
}

int process_rooms(char *params, client *player) {
    int i;

    printf("Processing ROOMS\n");
    if (params || !player) return ERROR; // message ROOMS has no params

    // No rooms
    if (rooms_counter == 0) {
        sprintf(resp_buff, get_response(ROOMS_ERR));
        if (send_message(player->socket, TRUE)) return ERROR;
    }
    else {
        for (i = 0; i < MAX_ROOMS; i++) {
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

int join_into_found_room(room *room, client *player) {
    if (!room || !player) return ERROR;

    room->player2 = player;
    room->room_state = FULL;
    player->room_number = room->number;
    player->is_player1 = FALSE;

    if (room->player1->state == ST_DISCONNECTED) {
        sprintf(resp_buff, get_response(FIND_OK_OPP_LOST_CON), room->number, room->player1->nickname);
        if (send_message(player->socket, TRUE)) return ERROR;
    }
    else {
        sprintf(resp_buff, get_response(FIND_OK_OPP_READY), room->number, room->player1->nickname);
        if (send_message(player->socket, TRUE)) return ERROR;

        sprintf(resp_buff, get_response(OPP_CON), player->nickname);
        if (send_message(room->player1->socket, TRUE)) return ERROR;
    }
    player->state = ST_OPP_TURN;
    inform_clients_in_lobby(room->number, UPDATE, player->nickname);
    return NO_ERROR;
}

int process_find(char *params, client *player) {
    int i;

    printf("Processing FIND\n");
    if (params || !player) return ERROR; // message FIND has no params

    for (i = 0; i < MAX_ROOMS; i++) {
        if (!rooms[i]) continue;

        if (rooms[i]->room_state == FREE) {
            if (join_into_found_room(rooms[i], player)) return ERROR;
            return NO_ERROR;
        }
    }

    sprintf(resp_buff, get_response(FIND_ERR));
    if (send_message(player->socket, TRUE)) return ERROR;

    return NO_ERROR;
}

int process_create(char *params, client *player) {
    int i;

    printf("Processing CREATE\n");
    if (params || !player) return ERROR; // message CREATE has no params

    if (rooms_counter == MAX_ROOMS) {
        sprintf(resp_buff, get_response(CREATE_ERR));
        if (send_message(player->socket, TRUE)) return ERROR;
        return NO_ERROR;
    }

    for (i = 0; i < MAX_ROOMS; i++) {
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

int process_join(char *params, client *player) {
    int number;
    char *par, *rest;

    printf("Processing JOIN\n");
    if (!params || !player) return ERROR;

    par = strtok(params, END_OF_LINE);
    if (!par || (strlen(par) == 0)) return ERROR;

    number = strtol(par, &rest, 10);
    if (*rest != '\0') return ERROR;

    if (number < 0 || number > (MAX_ROOMS - 1)) {
        sprintf(resp_buff, get_response(JOIN_ERR_INV_NUM));
        if (send_message(player->socket, TRUE)) return ERROR;
        return NO_ERROR;
    }

    if (rooms[number] == NULL) {
        sprintf(resp_buff, get_response(JOIN_ERR_INV_ROOM));
        if (send_message(player->socket, TRUE)) return ERROR;
        return NO_ERROR;
    }

    if (rooms[number]->room_state == FULL) {
        sprintf(resp_buff, get_response(JOIN_ERR_FULL_ROOM));
        if (send_message(player->socket, TRUE)) return ERROR;
        return NO_ERROR;
    }

    rooms[number]->player2 = player;
    rooms[number]->room_state = FULL;
    player->room_number = number;
    player->is_player1 = FALSE;

    if (rooms[number]->player1->state == ST_DISCONNECTED) {
        sprintf(resp_buff, get_response(JOIN_OK_OPP_LOST_CON), number, rooms[number]->player1->nickname);
        if (send_message(player->socket, TRUE)) return ERROR;
    }
    else {
        sprintf(resp_buff, get_response(JOIN_OK_OPP_READY), number, rooms[number]->player1->nickname);
        if (send_message(player->socket, TRUE)) return ERROR;

        sprintf(resp_buff, get_response(OPP_CON), player->nickname);
        if (send_message(rooms[number]->player1->socket, TRUE)) return ERROR;
    }

    player->state = ST_OPP_TURN;
    inform_clients_in_lobby(number, UPDATE, player->nickname);

    return NO_ERROR;
}

int process_logout(char *params, client *player) {

    printf("Processing LOGOUT\n");
    if (params || !player) return ERROR; // message LOGOUT has no params

    sprintf(resp_buff, get_response(LOGOUT_OK), player->nickname);
    if (send_message(player->socket, TRUE)) return ERROR;
    disconnect_client(player, NOW);

    return NO_ERROR;
}

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

void exit_room(client *player) {
    int number = player->room_number;
    remove_room(rooms[number]);
    rooms[number] = NULL;
    player->room_number = NOT_IN_ROOM;
    rooms_counter--;
    inform_clients_in_lobby(number, CLR, NULL);
}

int process_leave(char *params, client *player) {
    int number;
    client *opp;

    printf("Processing LEAVE\n");
    if (params || !player) return ERROR; // message LEAVE has no params

    number = player->room_number;
    // if in state WAITING_FOR_OPP
    if (player->state == ST_WAITING_FOR_OPP) {
        exit_room(player);
    }
    else {
        opp = get_opponent(rooms[number], player);
        if (opp->state != ST_DISCONNECTED) {
            sprintf(resp_buff, get_response(OPP_LEAVE));
            send_message(get_opponent(rooms[number], player)->socket, TRUE);
        }
        exit_room(player);
    }

    sprintf(resp_buff, get_response(LEAVE_OK));
    if (send_message(player->socket, TRUE)) return ERROR;
    player->state = ST_LOBBY;
    return NO_ERROR;
}

int process_turn(char *params, client *player) {
    char *part, *rest, *rest2;
    client *opp;
    int pos1, pos2;
    response ret;

    printf("Processing TURN\n");
    if (!params || !player) return ERROR;

    part = strtok_r(params, SEMICOLON, &rest);
    if (!part) return ERROR;

    if (strcmp(part, "SET") == 0) {
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
        ret = set_stone(rooms[player->room_number], player, pos1);

        switch (ret) {
            case TURN_OK_MILL:
                sprintf(resp_buff, get_response(TURN_OK_MILL));
                if (send_message(player->socket, TRUE)) return ERROR;

                sprintf(resp_buff, get_response(OPP_TURN_SET_MILL), pos1);
                opp = get_opponent(rooms[player->room_number], player);
                if (send_message(opp->socket, TRUE)) return ERROR;
                player->state = ST_TAKING_STONE;
                break;
            case TURN_OK_MILL_NO_TAKE:
                sprintf(resp_buff, get_response(TURN_OK_MILL_NO_TAKE));
                if (send_message(player->socket, TRUE)) return ERROR;

                sprintf(resp_buff, get_response(OPP_TURN_SET_MILL_NO_TAKE), pos1);
                opp = get_opponent(rooms[player->room_number], player);
                if (send_message(opp->socket, TRUE)) return ERROR;
                player->state = ST_OPP_TURN;
                break;
            case TURN_OK_NOT_MILL:
                sprintf(resp_buff, get_response(TURN_OK_NOT_MILL));
                if (send_message(player->socket, TRUE)) return ERROR;

                sprintf(resp_buff, get_response(OPP_TURN_SET_NOT_MILL), pos1);
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
    else if (strcmp(part, "SHIFT") == 0) {
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

        ret = shift_stone(rooms[player->room_number], player, pos1, pos2);

        switch (ret) {
          case TURN_OK_MILL:
              sprintf(resp_buff, get_response(TURN_OK_MILL));
              if (send_message(player->socket, TRUE)) return ERROR;

              sprintf(resp_buff, get_response(OPP_TURN_SHIFT_MILL), pos1, pos2);
              opp = get_opponent(rooms[player->room_number], player);
              if (send_message(opp->socket, TRUE)) return ERROR;
              player->state = ST_TAKING_STONE;
              break;
          case TURN_OK_MILL_NO_TAKE:
              sprintf(resp_buff, get_response(TURN_OK_MILL_NO_TAKE));
              if (send_message(player->socket, TRUE)) return ERROR;

              sprintf(resp_buff, get_response(OPP_TURN_SHIFT_MILL_NO_TAKE), pos1, pos2);
              opp = get_opponent(rooms[player->room_number], player);
              if (send_message(opp->socket, TRUE)) return ERROR;
              player->state = ST_OPP_TURN;
              break;
          case TURN_OK_NOT_MILL:
              sprintf(resp_buff, get_response(TURN_OK_NOT_MILL));
              if (send_message(player->socket, TRUE)) return ERROR;

              sprintf(resp_buff, get_response(OPP_TURN_SHIFT_NOT_MILL), pos1, pos2);
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
    else {
        printf("%s\n", part);
        return ERROR;
    }

    return NO_ERROR;
}

int process_take_stone(char *params, client *player) {
    char *rest;
    int pos, ret;
    client *opp;

    printf("Processing TAKE_STONE\n");
    if (!params || !player) return ERROR;

    pos = strtol(params, &rest, 10);
    if (*rest != '\0') return ERROR;

    if (pos < 0 || pos > 23) {
        sprintf(resp_buff, get_response(TK_STONE_ERR_INV_POS));
        if (send_message(player->socket, TRUE)) return ERROR;
        return NO_ERROR;
    }

    ret = take_stone(rooms[player->room_number], player, pos);

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


int process_opp_con_ok(char *params, client *player) {

    printf("Processing OPP_CON_OK\n");
    if (params || !player) return ERROR;
    player->state = ST_MY_TURN;

    return NO_ERROR;
}

int process_opp_turn_ok(char *params, client *player) {

    printf("Processing OPP_TURN_OK\n");
    if (!params || !player) return ERROR;

    if (strcmp(params, "NOT_MILL") == 0)
        player->state = ST_MY_TURN;
    else if (strcmp(params, "MILL") == 0)
        player->state = ST_OPP_TAKING_STONE;
    else if (strcmp(params, "MILL_NO_TAKE") == 0)
        player->state = ST_MY_TURN;
    else
        return ERROR;

    return NO_ERROR;
}

int process_opp_take_stone_ok(char *params, client *player) {

    printf("Processing OPP_TAKE_STONE_OK\n");
    if (!params || !player) return ERROR;

    if (strcmp(params, "CONTINUE") == 0)
        player->state = ST_MY_TURN;
    else if (strcmp(params, "LOOSE") == 0)
        player->state = ST_LOBBY;
    else
        return ERROR;

    return NO_ERROR;
}

int process_opp_leave_ok(char *params, client *player) {

    printf("Processing OPP_LEAVE_OK\n");
    if (params || !player) return ERROR;
    player->state = ST_LOBBY;

    return NO_ERROR;
}

int process_opp_lost_con_ok(char *params, client *player) {

    printf("Processing OPP_LOST_CON_OK\n");
    if (params || !player) return ERROR;

    if ((player->state == ST_MY_TURN) || (player->state == ST_TAKING_STONE))
        player->state = ST_OPP_LOST_CON;

    return NO_ERROR;
}

int process_opp_recon_ok(char *params, client *player) {
    return NO_ERROR;
}
int process_opp_discon_ok(char *params, client *player) {
    return NO_ERROR;
}
int process_update_room_ok(char *params, client *player) {
    return NO_ERROR;
}
int process_ping_ok(char *params, client *player) {
    return NO_ERROR;
}

/*
param remove 0 - store the client for possible reconnection, 1 - remove client immedietly
*/
void disconnect_client(client *client, int remove) {
    int i;

    // Get info about disconnected client
    getpeername(client->socket, (struct sockaddr *) &remote_addr, &len_addr);
    printf("Client %s %d was disconnected.\n", inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port));
    close(client->socket);
    clients[client->id] = NULL;
    if (remove == LATER) {
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (!discon_clients[i]) {
                discon_clients[i] = client;
                discon_clients_counter++;
            }
        }
    }
    else { // remove client
        remove_client(client);
        clients_counter--;
    }
}

int send_message(int client_socket, int clear_buff) {
    if (client_socket < 0) return ERROR;
    if (send(client_socket, resp_buff, strlen(resp_buff), 0) <= 0) {
        printf("Sending response failed.\n");
        return ERROR;
    }
    printf("SendTo(%d): %s", client_socket, resp_buff);
    if (clear_buff == TRUE)
        memset(resp_buff, 0, sizeof(resp_buff));
    return NO_ERROR;
}

/* Receive a message from the client
   and store it into the buffer
*/
int read_message(client *client) {
    message *msg;
    int ret_value;

    if (!client) return ERROR;

    memset(recv_buff, 0, sizeof(recv_buff)); // Clear the buffer
    if (recv(client->socket, recv_buff, sizeof(recv_buff), 0) <= 0) {
        return ERROR;
    }
    printf("RecvFrom(%d): %s", client->socket, recv_buff);
    msg = parse_msg(recv_buff);
    if (!msg) return ERROR;

//    printf("msg_id: %d\nmsg_name: %s\ndata: %s\n", msg->id, msg->name, msg->params);

    // if received message is allowed in the current client state
    if (verify_transition(client->state, msg->id) == ST_INVALID) {
         printf("Invalid event in the current state.\n");
        return ERROR;
    }
  //  printf("State before: ");
  //  print_state(client->state);

    // process message from the client
    if (process_func[msg->id](msg->params, client) == ERROR) {
        printf("Processing message failed.\n");
        return ERROR;
    }

//    printf("State after: ");
//    print_state(client->state);

    free_message(&msg);

    return NO_ERROR;
}

int create_server(int port) {
    int par = 1;
    int i, sd, max_sd, bytes;
    int server_socket, client_socket;

    // Create server socket and set options
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char *) &par, sizeof(int))) {
        printf("Problems with server socket setting.\n");
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
    else {
        printf("Bind-OK\n");
    }

    // Waiting for incoming connections
    if (listen(server_socket, 5)) {
        printf("Listening failed.\n");
        return ERROR;
    }
    else {
        printf("Listen-OK\n");
    }

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

        // Waiting for some activity on one of the sockets
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

            printf("New client %s:%d (socket: %d) is connected.\n", inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port), client_socket);

            for (i = 0; i < MAX_CLIENTS; i++) {
                if (!clients[i]) {
                    clients[i] = create_client(inet_ntoa(remote_addr.sin_addr), client_socket, i);
                    clients_counter++;
                    break;
                }
            }
        }

        for (i = 0; i < MAX_CLIENTS; i++) {
            if (!clients[i]) continue;
            sd = clients[i]->socket;

            if (FD_ISSET(sd, &read_fds)) {
                // Count of bytes ready to read
                ioctl(sd, FIONREAD, &bytes);
                if (bytes > 0){
                    // Read message from the client
                    if (read_message(clients[i]) == ERROR) {
                        sprintf(resp_buff, get_response(UNKNOWN_MSG));
                        send_message(clients[i]->socket, TRUE);
                        disconnect_client(clients[i], NOW);
                    }
                }
                // Problems with client socket
                else {
                    // nahle odpojeni - je treba uchovat udaje o klientovy pro pripadne znovu pripojeni
                    disconnect_client(clients[i], NOW); // LATER
                }
            }
        }
    }
    close(server_socket);
    return NO_ERROR;
}

int main(int argc, char *argv[]) {
    int port;
    char *rest;

    if (argc < 2) {
        printf("Missing argument.\nUsage: server [port]\nport: <0 - 65536>\n");
        return EXIT_FAILURE;
    }

    port = strtol(argv[1], &rest, 10);
    if (*rest != '\0' || port < 0 || port > 65536) {
        printf("Invalid port.\nport: <0 - 65536>\n");
        return EXIT_FAILURE;
    }

    if (create_server(port) == ERROR) {
        printf("Problems with the server.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
