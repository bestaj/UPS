#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "message.h"

/* All possible responses to clients */
char *responses[] = {
    [UNKNOWN_MSG] = "UNKNOWN_MESSAGE;ERR;400\n",
    [INIT_OK] = "CONNECT;OK;0;Welcome in the game Mill.\n",
    [INIT_ERR] = "CONNECT;ERR;401;Server is full.\n",

    [LOGIN_NEW_OK] = "LOGIN_REPLY;OK;0\n",
    [LOGIN_EXIST_LOBBY] = "LOGIN_REPLY;OK;1;LOBBY\n",
    [LOGIN_EXIST_WAITING_FREE] = "LOGIN_REPLY;OK;1;WAITING_FOR_OPP;%d;NONE\n",
    [LOGIN_EXIST_WAITING_FULL_READY] = "LOGIN_REPLY;OK;1;WAITING_FOR_OPP;%d;%s;OPP_READY\n",
    [LOGIN_EXIST_WAITING_FULL_NOT_READY] = "LOGIN_REPLY;OK;1;WAITING_FOR_OPP;%d;%s;OPP_LOST_CON\n",

    [LOGIN_EXIST_ACTIVE_STATE_P1_READY] = "LOGIN_REPLY;OK;1;%s;%d;P1;%s;OPP_READY;%d;%d;0_%d;1_%d;2_%d;3_%d;4_%d;5_%d;6_%d;7_%d;8_%d;9_%d;10_%d;11_%d;12_%d;13_%d;14_%d;15_%d;16_%d;17_%d;18_%d;19_%d;20_%d;21_%d;22_%d;23_%d\n",
    [LOGIN_EXIST_ACTIVE_STATE_P1_NOT_READY] = "LOGIN_REPLY;OK;1;%s;%d;P1;%s;OPP_LOST_CON;%d;%d;0_%d;1_%d;2_%d;3_%d;4_%d;5_%d;6_%d;7_%d;8_%d;9_%d;10_%d;11_%d;12_%d;13_%d;14_%d;15_%d;16_%d;17_%d;18_%d;19_%d;20_%d;21_%d;22_%d;23_%d\n",
    [LOGIN_EXIST_ACTIVE_STATE_P2_READY] = "LOGIN_REPLY;OK;1;%s;%d;P2;%s;OPP_READY;%d;%d;0_%d;1_%d;2_%d;3_%d;4_%d;5_%d;6_%d;7_%d;8_%d;9_%d;10_%d;11_%d;12_%d;13_%d;14_%d;15_%d;16_%d;17_%d;18_%d;19_%d;20_%d;21_%d;22_%d;23_%d\n",
    [LOGIN_EXIST_ACTIVE_STATE_P2_NOT_READY] = "LOGIN_REPLY;OK;1;%s;%d;P2;%s;OPP_LOST_CON;%d;%d;0_%d;1_%d;2_%d;3_%d;4_%d;5_%d;6_%d;7_%d;8_%d;9_%d;10_%d;11_%d;12_%d;13_%d;14_%d;15_%d;16_%d;17_%d;18_%d;19_%d;20_%d;21_%d;22_%d;23_%d\n",
    [LOGIN_EXIST_OPP_LOST_CON_P1_READY] = "LOGIN_REPLY;OK;1;OPP_LOST_CON;%d;P1;%s;OPP_READY;%s;%d;%d;0_%d;1_%d;2_%d;3_%d;4_%d;5_%d;6_%d;7_%d;8_%d;9_%d;10_%d;11_%d;12_%d;13_%d;14_%d;15_%d;16_%d;17_%d;18_%d;19_%d;20_%d;21_%d;22_%d;23_%d\n",
    [LOGIN_EXIST_OPP_LOST_CON_P1_NOT_READY] = "LOGIN_REPLY;OK;1;OPP_LOST_CON;%d;P1;%s;OPP_LOST_CON;%s;%d;%d;0_%d;1_%d;2_%d;3_%d;4_%d;5_%d;6_%d;7_%d;8_%d;9_%d;10_%d;11_%d;12_%d;13_%d;14_%d;15_%d;16_%d;17_%d;18_%d;19_%d;20_%d;21_%d;22_%d;23_%d\n",
    [LOGIN_EXIST_OPP_LOST_CON_P2_READY] = "LOGIN_REPLY;OK;1;OPP_LOST_CON;%d;P2;%s;OPP_READY;%s;%d;%d;0_%d;1_%d;2_%d;3_%d;4_%d;5_%d;6_%d;7_%d;8_%d;9_%d;10_%d;11_%d;12_%d;13_%d;14_%d;15_%d;16_%d;17_%d;18_%d;19_%d;20_%d;21_%d;22_%d;23_%d\n",
    [LOGIN_EXIST_OPP_LOST_CON_P2_NOT_READY] = "LOGIN_REPLY;OK;1;OPP_LOST_CON;%d;P2;%s;OPP_LOST_CON;%s;%d;%d;0_%d;1_%d;2_%d;3_%d;4_%d;5_%d;6_%d;7_%d;8_%d;9_%d;10_%d;11_%d;12_%d;13_%d;14_%d;15_%d;16_%d;17_%d;18_%d;19_%d;20_%d;21_%d;22_%d;23_%d\n",
    [LOGIN_ERR_NICK_EXIST] = "LOGIN_REPLY;ERR;401;nickname (%s) already exists\n",
    [LOGIN_ERR_NICK_WITH_SEMICOLON] = "LOGIN_REPLY;ERR;402;nickname contains semicolon\n",
    [LOGIN_ERR_NICK_WHITESPACE] = "LOGIN_REPLY;ERR;403;nickname starts/ends with whitespace\n",
    [LOGIN_ERR_NICK_TOO_LONG] = "LOGIN_REPLY;ERR;404;nickname is too long\n",
    [LOGIN_ERR_CLIENT_NOT_FOUND] = "LOGIN_REPLY;ERR;405;client was not found\n",
    [LOGIN_ERR_GAME_IS_OVER] = "LOGIN_REPLY;ERR;406;game is over\n",

    [ROOMS_OK_FREE] = "ROOMS_REPLY;OK;0;%d;%s\n",
    [ROOMS_OK_FULL] = "ROOMS_REPLY;OK;1;%d;%s;%s\n",
    [ROOMS_ERR] = "ROOMS_REPLY;ERR;401;no rooms\n",

    [FIND_OK_OPP_READY] = "FIND_REPLY;OK;0;%d;%s;OPP_READY\n",
    [FIND_OK_OPP_LOST_CON] = "FIND_REPLY;OK;0;%d;%s;OPP_LOST_CON\n",
    [FIND_ERR] = "FIND_REPLY;ERR;401;no free room\n",

    [CREATE_OK] = "CREATE_REPLY;OK;0;%d\n",
    [CREATE_ERR] = "CREATE_REPLY;ERR;401;maximum rooms reached\n",

    [JOIN_OK_OPP_READY] = "JOIN_REPLY;OK;0;%d;%s;OPP_READY\n",
    [JOIN_OK_OPP_LOST_CON] = "JOIN_REPLY;OK;0;%d;%s;OPP_LOST_CON\n",
    [JOIN_ERR_INV_NUM] = "JOIN_REPLY;ERR;401;invalid room number\n",
    [JOIN_ERR_INV_ROOM] = "JOIN_REPLY;ERR;402;room does not exist\n",
    [JOIN_ERR_FULL_ROOM] = "JOIN_REPLY;ERR;403;room %d is full\n",

    [LOGOUT_OK] = "LOGOUT_REPLY;OK;0\n",

    [LEAVE_OK] = "LEAVE_REPLY;OK;0\n",

    [TURN_OK_MILL] = "TURN_REPLY;OK;0;MILL\n",
    [TURN_OK_MILL_TAKE_ANY_STONE] = "TURN_REPLY;OK;1;MILL_TAKE_ANY_STONE\n",
    [TURN_OK_NOT_MILL_WIN] = "TURN_REPLY;OK;2;NOT_MILL;WIN\n",
    [TURN_OK_NOT_MILL_CONT] = "TURN_REPLY;OK;2;NOT_MILL;CONTINUE\n",
    [TURN_ERR_CANNOT_SET] = "TURN_REPLY;ERR;401;no stones to set\n",
    [TURN_ERR_CANNOT_SHIFT] = "TURN_REPLY;ERR;402;cannot shift stone yet\n",
    [TURN_ERR_INV_POS1] = "TURN_REPLY;ERR;403;invalid position 1\n",
    [TURN_ERR_INV_POS2] = "TURN_REPLY;ERR;403;invalid position 2\n",
    [TURN_ERR_FULL_POS] = "TURN_REPLY;ERR;404;position %d is full\n",
    [TURN_ERR_MISSING_STONE] = "TURN_REPLY;ERR;405;missing stone on position 1\n",
    [TURN_ERR_WRONG_POS2] = "TURN_REPLY;ERR;406;cannot shift stone to position 2\n",

    [TK_STONE_OK_WIN] = "TAKE_STONE_REPLY;OK;0;WIN\n",
    [TK_STONE_OK_CONT] = "TAKE_STONE_REPLY;OK;0;CONTINUE\n",
    [TK_STONE_ERR_INV_POS] = "TAKE_STONE_REPLY;ERR;401;invalid position\n",
    [TK_STONE_ERR_MISSING_STONE] = "TAKE_STONE_REPLY;ERR;402;missing opponents stone\n",
    [TK_STONE_ERR_IN_MILL] = "TAKE_STONE_REPLY;ERR;403;taking stone is in mill\n",

    [OPP_CON] = "OPP_CON;INFO;100;%s\n",
    [OPP_TURN_SET_MILL] = "OPP_TURN;INFO;101;SET;%d;MILL\n",
    [OPP_TURN_SET_NOT_MILL_LOOSE] = "OPP_TURN;INFO;101;SET;%d;NOT_MILL;LOOSE\n",
    [OPP_TURN_SET_NOT_MILL_CONT] = "OPP_TURN;INFO;101;SET;%d;NOT_MILL;CONTINUE\n",
    [OPP_TURN_SHIFT_MILL] = "OPP_TURN;INFO;101;SHIFT;%d;%d;MILL\n",
    [OPP_TURN_SHIFT_NOT_MILL_LOOSE] = "OPP_TURN;INFO;101;SHIFT;%d;%d;NOT_MILL;LOOSE\n",
    [OPP_TURN_SHIFT_NOT_MILL_CONT] = "OPP_TURN;INFO;101;SHIFT;%d;%d;NOT_MILL;CONTINUE\n",
    [OPP_TK_STONE_LOOSE] = "OPP_TAKE_STONE;INFO;102;%d;LOOSE\n",
    [OPP_TK_STONE_CONT] = "OPP_TAKE_STONE;INFO;102;%d;CONTINUE\n",
    [OPP_LEAVE] = "OPP_LEAVE;INFO;103\n",
    [OPP_LOST_CON] = "OPP_LOST_CON;INFO;104\n",
    [OPP_RECON] = "OPP_RECON;INFO;105\n",
    [OPP_DISCON] = "OPP_DISCON;INFO;106\n",

    [UPD_ROOM_CLR] = "UPDATE_ROOM;INFO;107;%d;CLR\n",
    [UPD_ROOM_ADD] = "UPDATE_ROOM;INFO;107;%d;ADD;%s\n",
    [UPD_ROOM_UPDATE] = "UPDATE_ROOM;INFO;107;%d;UPDATE;%s\n",
    [PING] = "PING;INFO;108\n"
};

/* Possible type of messages from the clients
  Index to the array response to ID of the message
*/
char *list_of_messages[] = {
    "CONNECT_OK", // 0
    "LOGIN", // 1
    "ROOMS", // 2
    "FIND", // 3
    "CREATE", // 4
    "JOIN", // 5
    "LOGOUT", // 6
    "LEAVE", // 7
    "TURN", // 8
    "TAKE_STONE", // 9
    "OPP_CON_OK", // 10
    "OPP_TURN_OK", // 11
    "OPP_TAKE_STONE_OK", // 12
    "OPP_LEAVE_OK", // 13
    "OPP_LOST_CON_OK", // 14
    "OPP_RECON_OK", // 15
    "OPP_DISCON_OK", // 16
    "UPDATE_ROOM_OK", // 17
    "PING_OK" // 18
};

// if semicolon should be after message name in message from the client, 1 = true, 0 = false
short semicolon_after_msg_name[] = {
  0, // CONNECT_OK
  1, // LOGIN
  0, // ROOOMS
  0, // FIND
  0, // CREATE
  1, // JOIN
  0, // LOGOUT
  0, // LEAVE
  1, // TURN
  1, // TAKE_STONE
  0, // OPP_CON_OK
  1, // OPP_TURN_OK
  1, // OPP_TAKE_STONE_OK
  0, // OPP_LEAVE_OK
  0, // OPP_LOST_CON_OK
  0, // OPP_RECON_OK
  0, // OPP_DISCON_OK
  1, // UPDATE_ROOM_OK
  0 // PING_OK
};

/* Return the particular response */
char *get_response(response resp) {
    return responses[resp];
}

/* Process the received message from the client */
message *parse_msg(char *data) {
    message *msg;
    char *part, *rest;
    int id;

    // Get message ID
    part = strtok(data, SEMICOLON);
    if (!part) return NULL;

    // Convert msg_id on number
    id = strtol(part, &rest, 10);
    if (*rest != '\0' || id < 0 || id > 19) {
        printf("Invalid msg_id\n");
        return NULL;
    }

    // Get message name
    if (semicolon_after_msg_name[id] == 0)
        part = strtok(NULL, END_OF_LINE);
    else
        part = strtok(NULL, SEMICOLON);

    if (!part) return NULL;

    // Verify if message ID correspond with message name
    if ((strlen(part) != strlen(list_of_messages[id])) && (strcmp(list_of_messages[id], part) != 0)) {
        printf("Invalid msg_name\n");
        return NULL;
    }

    msg = (message *) malloc(sizeof(message));
    if (!msg) return NULL;

    msg->id = id;
    msg->name = (char *) calloc(strlen(part), sizeof(char));
    if (!(msg->name)) return NULL;
    memcpy(msg->name, part, strlen(part));

    msg->params = NULL;
    part = strtok(NULL, END_OF_LINE);
    if (part) {
        if (strlen(part) > MSG_MAX_LENGTH)
            return NULL;
        msg->params = (char *) calloc(strlen(part), sizeof(char));
        memcpy(msg->params, part, strlen(part));
    }

    return msg;
}

/* Clear the message */
void free_message(message **msg) {
    if (!*msg) return;

    if ((*msg)->name) {
        free((*msg)->name);
    }
    if ((*msg)->params) {
        free((*msg)->params);
    }

    free(*msg);
    *msg = NULL;
}
