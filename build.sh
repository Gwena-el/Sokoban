#!/bin/bash
if g++ sokoban.cpp src/editor.cpp src/entity.cpp -std=c++17 -lX11 -lGL -lpthread -lpng -o sokoban ;
then ./sokoban ;
fi
