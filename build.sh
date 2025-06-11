#!/bin/bash
set -e
set -x
gcc -O2 -g -fvisibility=hidden -Wall -Wextra -Wpedantic -Werror -shared -o libdropout_glfw_stubs.so impl.c
