#!/bin/bash

if [ "$1" == "1" ]; then
    make s
elif [ "$1" == "0" ]; then
    make c
elif [ "$1" == "2" ]; then
    make d
else
    echo "Usage: ./lab05 [0|1]  (0 = client, 1 = server, 2 = demo)"
fi