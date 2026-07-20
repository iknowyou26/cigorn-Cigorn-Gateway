cmake_minimum_required(VERSION 3.10)

project(cigorn C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PostgreSQL REQUIRED)

include_directories(
    .
    ./polarssl
    ./tinyxml
    ${PostgreSQL_INCLUDE_DIRS}
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
target_compile_features(cigorn PRIVATE cxx_std_17)
target_include_directories(cigorn PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PostgreSQL_INCLUDE_DIRS}
)

if(WIN32)
    target_link_libraries(cigorn PRIVATE
        ${PostgreSQL_LIBRARIES}
        odbc32
        ws2_32
    )
else()
    target_compile_options(cigorn PRIVATE -m32)
    target_link_options(cigorn PRIVATE -m32)

    target_link_libraries(cigorn PRIVATE
        ${PostgreSQL_LIBRARIES}
        crypt
        pthread
    )
endif()
if(WIN32)
    target_link_libraries(cigorn PRIVATE ws2_32 iphlpapi)
endif()