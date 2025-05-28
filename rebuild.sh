#!/bin/sh

set -e

CFLAGS='-std=c89 -g -Wall -Wextra -Werror -pedantic -I . -Wno-format'

ex() {
    printf "  $@"
    "$@"
    local ret=$?
    if [ "$ret" = 0 ]
    then
        printf "\r+\n"
    else
        printf "\r-\n"
        exit $ret
    fi
}

compile() {
    cc ${CFLAGS} -o "$@"
}

ex compile _opsgen _opsgen.c
ex ./_opsgen

ex compile build/main main.c
ex compile build/sas  sas.c

ex rm _opsgen
