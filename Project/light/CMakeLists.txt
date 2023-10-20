cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

set(target "yunyun")

project(${target}
    LANGUAGES C CXX
    VERSION 1.0.0.1
    )

# 默认debug版本
if(NOT U_BUILD_TYPE)
    set(U_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()
if(${U_BUILD_TYPE} STREQUAL "Debug")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    # set(CMAKE_CONFIGURATION_TYPES Debug CACHE STRING "Build type" FORCE)
else()
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    # set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "Build type" FORCE)
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CONFIGURATION_TYPES: ${CMAKE_CONFIGURATION_TYPES}")

# CMAKE_INSTALL_PREFIX 自定义安装位置，cmake的所有安装地址都是相对于这个地址的
# 用户可以使用cmake -DCMAKE_INSTALL_PREFIX=xxx指定
# 默认情况：Windows上的C:\Program Files和Unix上的/usr/local
# 用户通过U_INSTALL_PREFIX指定安装目录前缀
if(U_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX ${U_INSTALL_PREFIX})
else()
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/cmake_ign_install")
endif()
message(STATUS "Project will be installed to ${CMAKE_INSTALL_PREFIX}")

add_executable(${target} "")

target_sources(${target}
    PRIVATE
        light_yy.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../../depends/glad/src/glad.c
)

target_include_directories(${target}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../depends/glad/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../depends/glfw/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../include>
)

set_target_properties(${target}
    PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
)

target_link_directories(${target}
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/../../depends/glfw/lib-vc2019
)

target_link_libraries(${target}
    PRIVATE
        glfw3
        opengl32
)

# windows 无法设置rpath，我们直接拷贝到同一个目录
# 执行没有输出的custom command
# 当声明了库或可执行目标，就可以使用add_custom_command将其他命令锁定到目标上。这些命令将在特定的时间执行，与它们所附加的目标的执行相关联。
# CMake通过以下选项，定制命令执行顺序:
# PRE_BUILD：在执行与目标相关的任何其他规则之前执行的命令。但能在Visual Studio 7或更高版本进行。
# PRE_LINK：使用此选项，命令在编译目标之后，调用链接器或归档器之前执行。Visual Studio 7或更高版本之外的生成器中使用PRE_BUILD将被解释为PRE_LINK。
# POST_BUILD：如前所述，这些命令将在执行给定目标的所有规则之后运行。
list(APPEND depend_libs "${CMAKE_CURRENT_LIST_DIR}/../../depends/glfw/lib-vc2019/glfw3.dll")
foreach(_file IN LISTS depend_libs)
    add_custom_command(
        TARGET 
            ${target}
        POST_BUILD
        COMMAND
            ${CMAKE_COMMAND} ARGS -E copy ${_file} $<TARGET_FILE_DIR:${target}>
        COMMENT "Copy dynamic lib: ${_file} of imported to output dir: $<TARGET_FILE_DIR:${target}>"
    )
endforeach()
