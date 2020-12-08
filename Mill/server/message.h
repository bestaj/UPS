#ifndef __MESSAGE__
#define __MESSAGE__

#define SEMICOLON ";"
#define END_OF_LINE "\n"
#define MSG_MAX_LENGTH 128

typedef enum e_response {
    UNKNOWN_MSG,
    INIT_OK,
    INIT_ERR,

    LOGIN_NEW_OK,
    LOGIN_EXIST_LOBBY,
    LOGIN_EXIST_WAITING_FREE,
    LOGIN_EXIST_WAITING_FULL_READY,
    LOGIN_EXIST_WAITING_FULL_NOT_READY,
    LOGIN_EXIST_ACTIVE_STATE_P1_READY,
    LOGIN_EXIST_ACTIVE_STATE_P1_NOT_READY,
    LOGIN_EXIST_ACTIVE_STATE_P2_READY,
    LOGIN_EXIST_ACTIVE_STATE_P2_NOT_READY,

    LOGIN_EXIST_MY_TURN_P1_READY,
    LOGIN_EXIST_MY_TURN_P1_NOT_READY,
    LOGIN_EXIST_MY_TURN_P2_READY,
    LOGIN_EXIST_MY_TURN_P2_NOT_READY,
    LOGIN_EXIST_OPP_TURN_P1_READY,
    LOGIN_EXIST_OPP_TURN_P1_NOT_READY,
    LOGIN_EXIST_OPP_TURN_P2_READY,
    LOGIN_EXIST_OPP_TURN_P2_NOT_READY,
    LOGIN_EXIST_TAKING_P1_READY,
    LOGIN_EXIST_TAKING_P1_NOT_READY,
    LOGIN_EXIST_TAKING_P2_READY,
    LOGIN_EXIST_TAKING_P2_NOT_READY,
    LOGIN_EXIST_OPP_TAKING_P1_READY,
    LOGIN_EXIST_OPP_TAKING_P1_NOT_READY,
    LOGIN_EXIST_OPP_TAKING_P2_READY,
    LOGIN_EXIST_OPP_TAKING_P2_NOT_READY,

    LOGIN_EXIST_OPP_LOST_CON_P1_READY,
    LOGIN_EXIST_OPP_LOST_CON_P1_NOT_READY,
    LOGIN_EXIST_OPP_LOST_CON_P2_READY,
    LOGIN_EXIST_OPP_LOST_CON_P2_NOT_READY,
    LOGIN_ERR_NICK_EXIST,
    LOGIN_ERR_NICK_WITH_SEMICOLON,
    LOGIN_ERR_NICK_WHITESPACE,
    LOGIN_ERR_NICK_TOO_LONG,
    LOGIN_ERR_CLIENT_NOT_FOUND,
    LOGIN_ERR_GAME_IS_OVER,

    ROOMS_OK_FREE,
    ROOMS_OK_FULL,
    ROOMS_ERR,

    FIND_OK_OPP_READY,
    FIND_OK_OPP_LOST_CON,
    FIND_ERR,

    CREATE_OK,
    CREATE_ERR,

    JOIN_OK_OPP_READY,
    JOIN_OK_OPP_LOST_CON,
    JOIN_ERR_INV_NUM,
    JOIN_ERR_INV_ROOM,
    JOIN_ERR_FULL_ROOM,

    LOGOUT_OK,

    LEAVE_OK,

    TURN_OK_MILL,
    TURN_OK_MILL_TAKE_ANY_STONE,
    TURN_OK_NOT_MILL_WIN,
    TURN_OK_NOT_MILL_CONT,
    TURN_ERR_CANNOT_SET,
    TURN_ERR_CANNOT_SHIFT,
    TURN_ERR_INV_POS1,
    TURN_ERR_INV_POS2,
    TURN_ERR_FULL_POS,
    TURN_ERR_MISSING_STONE,
    TURN_ERR_WRONG_POS2,

    TK_STONE_OK_WIN,
    TK_STONE_OK_CONT,
    TK_STONE_ERR_INV_POS,
    TK_STONE_ERR_MISSING_STONE,
    TK_STONE_ERR_IN_MILL,

    OPP_CON,
    OPP_TURN_SET_MILL,
    OPP_TURN_SET_NOT_MILL_LOOSE,
    OPP_TURN_SET_NOT_MILL_CONT,
    OPP_TURN_SHIFT_MILL,
    OPP_TURN_SHIFT_NOT_MILL_LOOSE,
    OPP_TURN_SHIFT_NOT_MILL_CONT,
    OPP_TK_STONE_LOOSE,
    OPP_TK_STONE_CONT,
    OPP_LEAVE,
    OPP_LOST_CON,
    OPP_RECON,
    OPP_DISCON,

    UPD_ROOM_CLR,
    UPD_ROOM_ADD,
    UPD_ROOM_UPDATE,
    PING
} response;

typedef struct themessage {
    int id;
    char *name;
    char *params;
} message;

char *get_response(response resp);

message *parse_msg(char *data);

void free_message(message **msg);

#endif
