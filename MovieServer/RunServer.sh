#!/bin/bash

make clean
make all
echo Running movie server................
Build/movie_server > /etc/movie_lists/MovieServer.log
