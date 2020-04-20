#!/bin/bash


./server -u some.sock &
spid=$!
sleep 1

./client --unix some.sock

wait $spid
