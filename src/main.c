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

#include "main.h"

int main(int argc, char** argv) {
    int opt = 0;

    while((opt = getopt(argc, argv, "v")) != -1) {
        switch(opt) {
            case 'v':
                printf("%s version: %s\r\n", argv[0], VERSION);
                return EXIT_SUCCESS;
                break;
            default: /* ? */
                fprintf(stderr, "Usage: %s [-v]\r\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    loop_context* context;
    context = malloc(sizeof(loop_context));

    return loop(context);
}
