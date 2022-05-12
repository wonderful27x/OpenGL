#!/bin/bash

gl_root_dir=`pwd`/../..
gl_include_dir=$gl_root_dir/include
gl_src_dir=$gl_root_dir/source
gl_libs_dir=$gl_root_dir/libs
assimp_lib_dir=$gl_libs_dir/assimp
glfw3_lib_dir=$gl_libs_dir/glfw
ffmpeg_include_dir=$HOME/wonderful/media/ffmpeg_build/include
ffmpeg_lib_dir=$HOME/wonderful/media/ffmpeg_build/lib

gladsrc=$gl_src_dir/glad.c
stb_src=$gl_src_dir/stb_image.cpp

echo g++ build... 
echo link lib dir: $gl_libs_dir

g++ -o run.out $1 $gladsrc $stb_src -I$gl_include_dir -I$ffmpeg_include_dir -L$ffmpeg_lib_dir -L$assimp_lib_dir -L$glfw3_lib_dir -lavcodec -lavdevice -lavformat -lavutil -lassimp -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXxf86vm -lXinerama -lXcursor -lXi -ldl -g --std=c++11

