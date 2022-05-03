
#!/bin/bash
gladsrc=`pwd`/../../glad/src/glad.c
echo $gladsrc
g++ -o run.out $1 $gladsrc -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXxf86vm -lXinerama -lXcursor -lXi -ldl

