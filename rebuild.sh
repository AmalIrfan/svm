#!/bin/sh

CFLAGS='-std=c89 -g -Wall -Wextra -Werror -pedantic -Wno-format  -I .'

if [ -t 1 ]; then
    CFLAGS="$CFLAGS -fdiagnostics-color=always"
fi

ex() {
    printf "  $*" >&2
    "$@" > tmp.out 2> tmp.err
    local ret=$?
    if [ "$ret" = 0 ]
    then
        printf "\r+\n" >&2
    else
        printf "\r-\n" >&2
    fi

    cat tmp.err >&2
    cat tmp.out
    rm tmp.out tmp.err

    if [ "$ret" != 0 ]
    then exit $ret
    fi
}

compile() {
    cc ${CFLAGS} -o "$@"
}

ex compile _opsgen _opsgen.c
ex ./_opsgen > output

ex compile build/main main.c
ex compile build/sas  sas.c

ex rm _opsgen
