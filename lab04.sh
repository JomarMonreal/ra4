#!/bin/bash

if [ "$1" == "1" ]; then
    if [ -n "$2" ]; then
        make s CPU=$2
    else
        make s
    fi
elif [ "$1" == "0" ]; then
    make c
elif [ "$1" == "2" ]; then
    make d
else
    echo "Usage: ./lab05 [0|1|2] [core]  (0 = client, 1 = server [core], 2 = demo)"
fi
