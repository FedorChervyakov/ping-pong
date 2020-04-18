#!/bin/bash

./server -u SOCKET_PATH &

./client --unix SOCKET_PATH &
