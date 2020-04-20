#!/bin/bash


timeout -k 5s 5s ./server -u some.sock &
sleep 1

./client --unix some.sock
