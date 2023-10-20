#!/bin/bash

echo "example: ./cmake-build-win.sh [2019] [Release] [Win32]"

declare -A msvc_versions
msvc_versions[2013]="Visual Studio 12 2013"
msvc_versions[2015]="Visual Studio 14 2015"
msvc_versions[2017]="Visual Studio 15 2017"
msvc_versions[2019]="Visual Studio 16 2019"
msvc_versions[2022]="Visual Studio 17 2022"

msvc_vs=2019
msvc_name=${msvc_versions[$msvc_vs]}
if [ $1 ]; then
    msvc_vs=$1
    msvc_name=${msvc_versions[$msvc_vs]}
fi
echo "msvc version: $msvc_name"

build_type=Debug
if [ $2 ]; then
    build_type=$2
fi
echo "cmake build type: $build_type"

build_arrch=WIN32
if [ $3 ]; then
    build_arrch=$3
fi
echo "cmake build arrch: $build_arrch"

if [ $msvc_vs -le 2017 ]; then
    if [ "$build_arrch" = "Win64" ]; then
        msvc_name="$msvc_name $build_arrch"
    fi
    build_arrch=""
fi

# config
# cmake -H. -B cmake_ign_build -G "$msvc_name"
cmake -H. -B cmake_ign_build -G "$msvc_name" -A "$build_arrch" \
    -D U_BUILD_TYPE=$build_type \
    # -D U_PLATFORM_OPTION_DEFINE=$platform_option_define \
    # -D U_TEST=$ctest \
    # -D U_SUPER_BUILD=ON \
    # -C cmake_preload_script.cmake

# build install package
cmake --build cmake_ign_build --config $build_type \
    # --target install --config $build_type \
    # --target package \
    # --target package_source \
    # -- VERBOSE=1

# package source
# cmake --build cmake_ign_build --target package 这种方法打包程序会使代码重新编译
# cmake --build cmake_ign_build --target package_source 这种方法打包源码有bug
# visual studio 中无法生成pacakage_source see -> https://gitlab.kitware.com/cmake/cmake/-/issues/13058
# cd cmake_ign_build
# cpack -G TGZ -C $build_type --config CPackConfig.cmake
# cpack -G ZIP -C $build_type --config CPackConfig.cmake
# cpack -G TGZ -C $build_type --config CPackSourceConfig.cmake
# cpack -G ZIP -C $build_type --config CPackSourceConfig.cmake

# test
# cmake --build cmake_ign_build --target test
# 使用-V可以获得可执行文件的输出
# cd cmake_ign_build
# ctest -V -C $build_type

./cmake_ign_build/$build_type/triangle.exe
