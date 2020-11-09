#ifndef __CLIENT__
#define __CLIENT__

#define MAX_LEN_NICK 18
#define MAX_LEN_ADDRESS 128
#define STATES_COUNT 11
#define EVENTS_COUNT 18
#define NOT_IN_ROOM -1
#define TRUE 1
#define FALSE 0

typedef enum e_state {
    ST_INVALID,
    ST_CONNECTED,
    ST_DISCONNECTED,
    ST_LOBBY,
    ST_WAITING_FOR_OPP,
    ST_MY_TURN,
    ST_OPP_TURN,
    ST_TAKING_STONE,
    ST_OPP_TAKING_STONE,
    ST_OPP_LOST_CON,
    ST_UNKNOWN
} state;

typedef enum e_event {
  EV_LOGIN = 0,
  EV_ROOMS = 1,
  EV_FIND = 2,
  EV_CREATE = 3,
  EV_JOIN = 4,
  EV_LOGOUT = 5,
  EV_LEAVE = 6,
  EV_TURN = 7,
  EV_TAKE_STONE = 8,
  EV_OPP_CON_OK = 9,
  EV_OPP_TURN_OK = 10,
  EV_OPP_TAKE_STONE_OK = 11,
  EV_OPP_LEAVE_OK = 12,
  EV_OPP_LOST_CON_OK = 13,
  EV_OPP_RECON_OK = 14,
  EV_OPP_DISCON_OK = 15,
  EV_UPDATE_ROOM_OK = 16,
  EV_PING_OK = 17,
} event;

typedef struct theclient {
    int socket;
    int room_number;
    int id;
    char address[MAX_LEN_ADDRESS];
    char nickname[MAX_LEN_NICK];
    state state;
    state previous_state;
    int is_player1;
} client;


int verify_transition(state st, int ev);

void print_state(state state);

client *create_client(char *address, int sd, int id);

void remove_client(client *client);

#endif
