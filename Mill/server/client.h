#ifndef __CLIENT__
#define __CLIENT__

#define MAX_LEN_NICK 18
#define MAX_LEN_ADDRESS 128
#define STATES_COUNT 12
#define EVENTS_COUNT 19
#define NOT_IN_ROOM -1
#define TRUE 1
#define FALSE 0

typedef enum e_state {
    ST_INVALID,
    ST_INIT,
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
  EV_CONNECT_OK = 18
} event;

typedef struct theclient {
    int socket; // socket of the client
    int room_number;  // number of the room in which is the client or NOT_IN_ROOM (-1) if client is not in the game
    int id;   // index to the array of clients
    pthread_t thread_id;  // thread id - used when client lost connection and waiting for possible reconnection
    char address[MAX_LEN_ADDRESS]; // address of the client
    char nickname[MAX_LEN_NICK];  // nickname of the client
    state state; // current state of the client
    state previous_state; // the last state in which client was in the game, when his opponent lost connection
    state last_state; // the last state of the client when lost connection
    int is_player1; // if the client is player 1
} client;


int verify_transition(state st, int ev);

char *get_state(state state);

client *create_client(char *address, int sd, int id);

void remove_client(client *client);

#endif
