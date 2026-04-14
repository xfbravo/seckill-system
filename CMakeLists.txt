cmake_minimum_required(VERSION 3.15)
project(seckill_system)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# AddressSanitizer for debugging memory issues
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

# 找依赖包
find_package(PkgConfig REQUIRED)
pkg_check_modules(HIREDIS required hiredis)
pkg_check_modules(YAML_CPP required yaml-cpp)
pkg_check_modules(MYSQL mysqlclient)

# 源码目录
aux_source_directory(src/common COMMON_SRC)
aux_source_directory(src/service COMMON_SRC)
aux_source_directory(src/repository COMMON_SRC)

# 头文件目录
include_directories(
    include
    ${HIREDIS_INCLUDE_DIRS}
    ${MYSQL_INCLUDE_DIRS}
    ${YAML_CPP_INCLUDE_DIRS}
    /usr/include/jsoncpp
)

add_executable(seckill_server
    src/main.cc
    src/http_server.cc
    ${COMMON_SRC}
)

# 链接
target_link_libraries(seckill_server
    jsoncpp
    hiredis
    yaml-cpp
    mysqlclient
    cpp-httplib
    pthread
    dl
)

# 安装
install(TARGETS seckill_server DESTINATION bin)
