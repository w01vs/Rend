#!/bin/bash

./build-Debug/Debug/Rend temp.rd

if [ $? -eq 0 ]; then
    if [ ! -f ./build-Debug/Debug/Rend ]; then
        echo "Error: ./Rend does not exist. Please ensure the executable is built and in the correct location."
        exit 1
    fi

    # Run actual exec
    ./rendout
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