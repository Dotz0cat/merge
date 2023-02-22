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

#ifndef LOOP_H
#define LOOP_H

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <event2/event.h>
#include <unistd.h>

#include <string.h>
#include <strings.h>

#define SIZE 9

#define ITEM_TABLE \
X(empty, none), \
X(blocked, none), \
X(item1, tier1), \
X(item2, tier2)

#define X(a, b) a
enum item_type {
    ITEM_TABLE
};
#undef X

enum item_tier {
    none,
    tier1,
    tier2
};

struct _location {
    int x;
    int y;
};

#define X(a, b) [a]=b
static const enum item_tier item_tier_lookup[] =
    {ITEM_TABLE};
#undef X

struct _item {
    struct _location location;

    enum item_type type;
    enum item_tier tier;
};

typedef struct _context loop_context;
typedef struct _event_box event_box;

struct _context {
    struct _item** board;

    event_box* events;
};

struct _event_box {
    struct event_base* base;
    struct event* signal_sigquit;
    struct event* signal_sigterm;
    struct event* signal_sigint;
    struct event* input_available;
    //struct event* signal_sighup;
    //struct event* signal_sigusr1;
    //struct event* signal_sigusr2;
};

int loop(loop_context* context);
static void sig_int_quit_term_cb(evutil_socket_t sig, short events, void* user_data);
struct _item** get_board(void);
void print_board(struct _item** board);
static void stdin_read_cb(evutil_socket_t event, short events, void* user_data);
static enum item_type type_promotion(enum item_type type);
static enum item_tier tier_promotion(enum item_tier tier);

static int error(struct _item* tile1, struct _item* tile2, struct _item** board);
static int merge_tiles(struct _item* tile1, struct _item* tile2, struct _item** board);
static int new_tile(struct _item* tile1, struct _item* tile2, struct _item** board);

#endif /* LOOP_H */
