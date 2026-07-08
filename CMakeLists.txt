cmake_minimum_required(VERSION 3.10)
project(cigorn C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")

include_directories(
    .
    ./polarssl
    ./tinyxml
    /usr/include/postgresql
)
file(GLOB CIGORN_CPP "*.cpp")
file(GLOB REPOSITORY_CPP "repositories/*.cpp")
file(GLOB TINYXML_CPP "tinyxml/*.cpp")
file(GLOB POLARSSL_C "polarssl/*.c")
file(GLOB ADAPTER_CPP "adapters/*.cpp")
add_executable(cigorn
    database/DatabaseFactory.cpp
    database/sqlserver/SQLServerDatabase.cpp
    ${CIGORN_CPP}
    ${REPOSITORY_CPP}
    ${TINYXML_CPP}
    ${POLARSSL_C}
    ${ADAPTER_CPP}
)

target_link_libraries(cigorn
    pq
    crypt
    pthread
)

