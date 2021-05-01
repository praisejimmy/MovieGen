#!/bin/bash

make clean
make all
echo Running movie server................
Build/movie_server > MovieLists/MovieServer.log 2>&1 &
