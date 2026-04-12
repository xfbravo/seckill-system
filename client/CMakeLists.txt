cmake_minimum_required(VERSION 3.16)
project(seckill_client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Network Widgets)

set(SOURCES
    main.cc
    MainWindow.cc
    ActivityDetailWindow.cc
    NetworkManager.cc
)

set(HEADERS
    MainWindow.h
    ActivityDetailWindow.h
    NetworkManager.h
)

add_executable(seckill_client WIN32
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(seckill_client
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::Widgets
)

if(WIN32)
    set_target_properties(seckill_client PROPERTIES
        WIN32_EXECUTABLE ON
    )
endif()
