#!/bin/bash

./build-Debug/Debug/comp temp.vx

if [ $? -eq 0 ]; then
    if [ ! -f ./rend ]; then
        echo "Error: ./rend does not exist. Please ensure the executable is built and in the correct location."
        exit 1
    fi

    ./rend
    status=$?
    echo "exit code: $status"

    if [ -z "$1" ]; then
        echo "Error: Expected exit code not provided. Usage: ./crun.sh <expected_exit_code>"
        exit 1
    fi

    echo "expected exit code: $1"
    if [ "$1" -eq "$status" ]; then
        echo "SUCCESS!!!!!"
    else
        echo "OOPS SOMETHING WENT WRONG"
    fi
else
    echo "Error: Compilation failed."
    exit 1
fi