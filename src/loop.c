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

    context->board = get_board();

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

struct _item** get_board(void) {
    struct _item** board;

    //0 will be replaced with condition
    if (0 != 1) {
        //initalize everything

        board = malloc(SIZE * sizeof(struct _item*));

        for (int i = 0; i < SIZE; i++) {

            board[i] = malloc(SIZE * sizeof(struct _item));

            for (int j = 0; j < SIZE; j++) {
                //board[i][j].location = 

                board[i][j].location.x = i;
                board[i][j].location.y = j;

                if ((i > 2 && i < 6) && (j > 2 && j < 6)) {
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
    printf("  0 1 2 3 4 5 6 7 8\r\n");
    for (int i = 0; i < SIZE; i++) {
        printf("%i ", i);
        for (int j = 0; j < SIZE; j++) {
            switch (board[i][j].type) {
                case (empty):
                    printf("0 ");
                break;
                case (blocked):
                    printf("* ");
                break;
                case (item1):
                    printf("a ");
                break;
                case (item2):
                    printf("b ");
                break;
            }
        }
        printf("\b\r\n");
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

    static const int (*commands[])(struct _item*, struct _item*, struct _item**) = {
        error,
        merge_tiles,
        new_tile
    };

    int res = 0;
    int command_number = 0;
    int locx1 = 0;
    int locy1 = 0;
    int locx2 = 0;
    int locy2 = 0;

    if (raw_command[0] != '\n') {
        char* hold = command;
        char* hold2;
        char* token;
        char* token2;

        token = strtok_r(hold, " ", &hold);

        if (strcasecmp(token, "merge") == 0) {
            command_number = 1;
        }
        else if (strcasecmp(token, "new") == 0) {
            command_number = 2;
        }

        token = strtok_r(hold, " ", &hold);

        if (token != NULL) {
            hold2 = token;

            token2 = strtok_r(hold2, ",", &hold2);

            locx1 = atoi(token2);

            token2 = strtok_r(hold2, ",", &hold2);

            locy1 = atoi(token2);

            token = strtok_r(hold, " ", &hold);

            if (token != NULL) {
                hold2 = token;

                token2 = strtok_r(hold2, ",", &hold2);

                locx2 = atoi(token2);

                token2 = strtok_r(hold2, ",", &hold2);

                locy2 = atoi(token2);
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

    res = (*commands[command_number])(tile1, tile2, context->board);

    if (res == 0) {
        printf("\033c");

        print_board(context->board);

        // if (raw_command[0] != '\n') {
        //     printf("%s\r\n", command);
        // }
    }

    free(raw_command);
    free(command);


    //sleep(1);

    event_add(context->events->input_available, NULL);
}

static enum item_type type_promotion(enum item_type type) {

    switch (type) {
        case (item1):
            return item2;
        break;
        case (item2):
            return item2;
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
            return tier2;
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

static int error(struct _item* tile1, struct _item* tile2, struct _item** board) {
    fprintf(stderr, "Invalid command\r\n");

    return 1;
}

static int merge_tiles(struct _item* tile1, struct _item* tile2, struct _item** board) {
    if (tile1 == NULL || tile2 == NULL) {
        fprintf(stderr, "Merge sent with wrong augments\r\n");
        return 1;
    }

    if (tile_compare(tile1, tile2)) {
        fprintf(stderr, "Cannot merge an item with itself\r\n");
        return 1;
    }

    enum item_type type1 = board[tile1->location.x][tile1->location.y].type;
    enum item_type type2 = board[tile2->location.x][tile2->location.y].type;

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

static int new_tile(struct _item* tile1, struct _item* tile2, struct _item** board) {
    (void) tile2;
    struct _location loc;
    loc.x = -1;
    loc.y = -1;

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
        // while (board[loc.x][loc.y].type != empty) {
        //     //generate
        // }
        loc.x = 5;
        loc.y = 5;
    }

    //randominous later
    if (board[loc.x][loc.y].type == empty) {
        board[loc.x][loc.y].type = item1;
        board[loc.x][loc.y].tier = item_tier_lookup[item1];
    }
    else {
        fprintf(stderr, "Invalid location\r\n");
        return 1;
    }

    return 0;
}
