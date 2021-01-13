#ifndef __ROOM__
#define __ROOM__

#define MAX_LEN_NICK 18
#define FULL 1
#define FREE 0
#define POSITIONS_COUNT 24

#define P1 1
#define P2 2

/* Structure of the room/game */
typedef struct theroom{
    int number;
    int room_state;
    client *player1;
    client *player2;

    int p1_unset_stones;
    int p2_unset_stones;
    int p1_stones;
    int p2_stones;
    int game_positions[POSITIONS_COUNT];
} room;

room *create_room(int number, client *player1);

void remove_room(room *room);

client *get_opponent(room *room, client *player);

response set_stone(room *room, client *player, int pos_id);

response shift_stone(room *room, client *player, int pos1_id, int pos2_id);

response take_stone(room *room, client *player, int pos_id);

#endif
