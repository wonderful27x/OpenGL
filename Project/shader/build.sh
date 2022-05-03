#!/bin/bash

gl_root_dir=`pwd`/../..
gl_include_dir=$gl_root_dir/include
gl_src_dir=$gl_root_dir/source

gladsrc=$gl_src_dir/glad.c

g++ -o run.out $1 $gladsrc -I$gl_include_dir -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXxf86vm -lXinerama -lXcursor -lXi -ldl

