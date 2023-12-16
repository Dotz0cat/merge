/*
Copyright 2023 Dotz0cat

This file is part of merge.

    merge is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    merge is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with merge.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "loop.h"

int loop(loop_context* context) {
    //events
    context->events = malloc(sizeof(event_box));
    context->events->base = event_base_new();

    if (context->events->base == NULL) {
        abort();
    }

    context->events->signal_sigquit = evsignal_new(context->events->base, SIGQUIT, sig_int_quit_term_cb, (void*) context);
    if (!context->events->signal_sigquit || event_add(context->events->signal_sigquit, NULL) < 0) abort();

    context->events->signal_sigterm = evsignal_new(context->events->base, SIGTERM, sig_int_quit_term_cb, (void*) context);
    if (!context->events->signal_sigterm || event_add(context->events->signal_sigterm, NULL) < 0) abort();

    context->events->signal_sigint = evsignal_new(context->events->base, SIGINT, sig_int_quit_term_cb, (void*) context);
    if (!context->events->signal_sigint || event_add(context->events->signal_sigint, NULL) < 0) abort();

    context->events->input_available = event_new(context->events->base, fileno(stdin), EV_READ, stdin_read_cb, (void*) context);
    if (!context->events->input_available  || event_add(context->events->input_available, NULL) < 0) abort();

    context->board = get_board(&context->upper_limit, &context->lower_limit);

    printf("\033c");

    print_board(context->board);

    event_base_loop(context->events->base, EVLOOP_NO_EXIT_ON_EMPTY);

    return 0;
}

static void sig_int_quit_term_cb(evutil_socket_t sig, short events, void* user_data) {
    loop_context* context = (loop_context*) user_data;

    struct timeval delay = {1, 0};

    event_base_loopexit(context->events->base, &delay);
}

struct _item** get_board(int* upper_limit, int* lower_limit) {
    struct _item** board;

    //0 will be replaced with condition
    if (0 != 1) {
        //initalize everything

        int upper;
        int lower;

        if (SIZE % 2 == 0) {
            upper = (SIZE / 2) + 1;
            lower = (SIZE / 2) - 2; 
        }
        else {
            //integer fun
            upper = (SIZE / 2) + 1;
            lower = (SIZE / 2) - 1;
        }

        *upper_limit = upper;
        *lower_limit = lower;


        board = malloc(SIZE * sizeof(struct _item*));

        for (int i = 0; i < SIZE; i++) {

            board[i] = malloc(SIZE * sizeof(struct _item));

            for (int j = 0; j < SIZE; j++) {
                //board[i][j].location = 

                board[i][j].location.x = i;
                board[i][j].location.y = j;

                if ((i >= lower && i <= upper) && (j >= lower && j <= upper)) {
                    board[i][j].type = empty;
                    board[i][j].tier = item_tier_lookup[empty];
                }
                else {
                    board[i][j].type = blocked;
                    board[i][j].tier = item_tier_lookup[blocked];
                }
            }
        }
    }

    return board;
}

void print_board(struct _item** board) {
    printf("%3c", ' ');
    for (int i = 0; i < SIZE; i++) {
        printf("%3i", i);
    }
    printf("\r\n");
    for (int i = 0; i < SIZE; i++) {
        printf("%2i ", i);
        for (int j = 0; j < SIZE; j++) {
            switch (board[i][j].type) {
                case (empty):
                    printf("%3c", '0');
                break;
                case (blocked):
                    printf("%3c", '*');
                break;
                case (item1):
                    printf("%3c", '1');
                break;
                case (item2):
                    printf("%3c", '2');
                break;
                case (item3):
                    printf("%3c", '3');
                break;
                case (item4):
                    printf("%3c", '4');
                break;
                case (item5):
                    printf("%3c", '5');
                break;
                case (item6):
                    printf("%3c", '6');
                break;
                case (item7):
                    printf("%3c", '7');
                break;
                case (item8):
                    printf("%3c", '8');
                break;
                case (item9):
                    printf("%3c", '9');
                break;
                case (item10):
                    printf("%3c", 'a');
                break;
                case (item11):
                    printf("%3c", 'b');
                break;
            }
        }
        printf("\r\n");
    }
}

static void stdin_read_cb(evutil_socket_t event, short events, void* user_data) {
    loop_context* context = (loop_context*) user_data;

    event_del(context->events->input_available);

    char* raw_command = malloc(256);
    fgets(raw_command, 255, stdin);
    
    char* command = malloc(256);

    if (raw_command[0] != '\n') {
        sscanf(raw_command, "%[^\n]", command);
    }

    static const int (*commands[])(struct _item*, struct _item*, loop_context* context) = {
        error,
        quit,
        merge_tiles,
        new_tile,
        unlock_new_tiles
    };

    int res = 0;
    int command_number = 0;
    int locx1 = -1;
    int locy1 = -1;
    int locx2 = -1;
    int locy2 = -1;

    if (raw_command[0] != '\n') {
        char* hold = command;
        char* hold2;
        char* token;
        char* token2;

        token = strtok_r(hold, " ", &hold);

        if (strcasecmp(token, "quit") == 0) {
            command_number = 1;
        }
        else if (strcasecmp(token, "merge") == 0) {
            command_number = 2;
        }
        else if (strcasecmp(token, "new") == 0) {
            command_number = 3;
        }
        else if (strcasecmp(token, "unlock") == 0) {
            command_number = 4;
        }

        token = strtok_r(hold, " ", &hold);

        if (token != NULL) {
            hold2 = token;

            token2 = strtok_r(hold2, ",", &hold2);

            sscanf(token2, "%i", &locx1);

            token2 = strtok_r(hold2, ",", &hold2);

            if (token2 == NULL) {
                locy1 = -1;
            }
            else {
                sscanf(token2, "%i", &locy1);
            }

            //sscanf(token, "%*[^0123456789]%i%*[^0123456789]%i", &locx1, &locy1);

            token = strtok_r(hold, " ", &hold);

            if (token != NULL) {
                hold2 = token;

                token2 = strtok_r(hold2, ",", &hold2);

                sscanf(token2, "%i", &locx2);

                token2 = strtok_r(hold2, ",", &hold2);

                if (token2 == NULL) {
                    locy2 = -1;
                }
                else {
                    sscanf(token2, "%i", &locy2);
                }
            }
        }
    }

    struct _item* tile1; 
    struct _item* tile2;

    //bound testing
    if (command_number < 0) {
        command_number = 0;
    }
    if (locx1 < 0 || locx1 > SIZE || locy1 < 0 || locy1 > SIZE) {
       tile1 = NULL;
    }
    else {
        tile1 = &context->board[locx1][locy1];
    }
    if (locx2 < 0 || locx2 > SIZE || locy2 < 0 || locy2 > SIZE) {
        tile2 = NULL;
    }
    else {
        tile2 = &context->board[locx2][locy2];
    }

    res = (*commands[command_number])(tile1, tile2, context);

    if (res == 0) {
        printf("\033c");

        print_board(context->board);

        // if (raw_command[0] != '\n') {
        //     printf("%s\r\n", command);
        // }
    }

    free(raw_command);
    free(command);

    event_add(context->events->input_available, NULL);
}

static enum item_type type_promotion(enum item_type type) {

    switch (type) {
        case (item1):
            return item2;
        break;
        case (item2):
            return item3;
        break;
        case (item3):
            return item4;
        break;
        case (item4):
            return item5;
        break;
        case (item5):
            return item6;
        break;
        case (item6):
            return item7;
        break;
        case (item7):
            return item8;
        break;
        case (item8):
            return item9;
        break;
        case (item9):
            return item10;
        break;
        case (item10):
            return item11;
        break;
        case (item11):
            return item11;
        break;
        default:
            return type;
        break;
    }
}

static enum item_tier tier_promotion(enum item_tier tier) {
    switch (tier) {
        case (tier1):
            return tier2;
        break;
        case (tier2):
            return tier3;
        break;
        case (tier3):
            return tier4;
        break;
        case (tier4):
            return tier5;
        break;
        case (tier5):
            return tier6;
        break;
        case (tier6):
            return tier7;
        break;
        case (tier7):
            return tier8;
        break;
        case (tier8):
            return tier9;
        break;
        case (tier9):
            return tier10;
        break;
        case (tier10):
            return tier11;
        break;
        case (tier11):
            return tier11;
        break;
        default:
            return tier;
        break;
    }
}

static int tile_compare(const struct _item* tile1, const struct _item* tile2) {
    int locx1 = tile1->location.x;
    int locy1 = tile1->location.y;

    int locx2 = tile2->location.x;
    int locy2 = tile2->location.y;

    if (locx1 == locx2 && locy1 == locy2) {
        //true they are equal
        return 1;
    }
    else {
        return 0;
    }
}

static int error(struct _item* tile1, struct _item* tile2, loop_context* context) {
    (void) tile1;
    (void) tile2;
    (void) context;

    fprintf(stderr, "Invalid command\r\n");

    return 1;
}

static int quit(struct _item* tile1, struct _item* tile2, loop_context* context) {
    (void) tile1;
    (void) tile2;

    struct timeval delay = {1, 0};

    event_base_loopexit(context->events->base, &delay);

    return 0;
}

static int merge_tiles(struct _item* tile1, struct _item* tile2, loop_context* context) {
    if (tile1 == NULL || tile2 == NULL) {
        fprintf(stderr, "Merge sent with wrong augments\r\n");
        return 1;
    }

    if (tile_compare(tile1, tile2)) {
        fprintf(stderr, "Cannot merge an item with itself\r\n");
        return 1;
    }

    enum item_type type1 = context->board[tile1->location.x][tile1->location.y].type;
    enum item_type type2 = context->board[tile2->location.x][tile2->location.y].type;

    if (type1 == blocked || type2 == blocked) {
        fprintf(stderr, "Blocked tiles cannot be merged\r\n");
        return 1;
    }
    else if (type1 == empty || type2 == empty) {
        fprintf(stderr, "Empty tiles cannot be merged\r\n");
        return 1;
    }
    else if (type1 != type2) {
        //fprintf(stderr, "%s is not the same type as %s\r\n", type1, type2);
        fprintf(stderr, "These types cannot be merged\r\n");
        return 1;
    }

    tile1->type = type_promotion(tile1->type);
    tile1->tier = tier_promotion(item_tier_lookup[tile1->type]);
    tile2->type = empty;
    tile2->tier = item_tier_lookup[tile2->type];


    return 0;
}

static int new_tile(struct _item* tile1, struct _item* tile2, loop_context* context) {
    (void) tile2;
    struct _location loc;

    if (tile1 != NULL) {
        if (tile1->type == empty) {
            loc = tile1->location;
        }
        else {
            fprintf(stderr, "You entered an invalid location\r\n");
            return 1;
        }
    }
    else {
        //gen later
        srand48(time(NULL));

        int tries = 0;
        do {
            loc.x = lrand48() % SIZE;
            loc.y = lrand48() % SIZE;
            tries++;
        } while (context->board[loc.x][loc.y].type != empty && tries < (SIZE*SIZE));
        
        if (tries == (SIZE*SIZE)) {
            fprintf(stderr, "The board is already full\r\n");
            return 1;
        }

    }

    //randominous later
    if (context->board[loc.x][loc.y].type == empty) {
        context->board[loc.x][loc.y].type = item1;
        context->board[loc.x][loc.y].tier = item_tier_lookup[item1];
    }
    else {
        fprintf(stderr, "Invalid location\r\n");
        return 1;
    }

    return 0;
}

static int unlock_new_tiles(struct _item* tile1, struct _item* tile2, loop_context* context) {
    (void) tile1;
    (void) tile2;

    int upper = context->upper_limit;
    int lower = context->lower_limit;

    upper++;
    lower--;

    if (upper > SIZE || lower < 0) {
        fprintf(stderr, "Already unlocked\r\n");
        return 1;
    }

    for (int i = lower; i <= upper; i++) {
        for (int j = lower; j <= upper; j++) {
            
            if (context->board[i][j].type == blocked) {
                context->board[i][j].type = empty;
            }
            
        }
    }

    context->upper_limit = upper;
    context->lower_limit = lower;

    return 0;
}
