cmake_minimum_required(VERSION 3.10)
project(VectorNumberSystem)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(number STATIC
        number.cpp
        number.h
)

add_library(vector SHARED
        vector.cpp
        vector.h
)
target_link_libraries(vector number)

add_executable(main main.cpp)
target_link_libraries(main number vector)

if(UNIX AND NOT APPLE)
    set_target_properties(vector PROPERTIES
            INSTALL_RPATH "$ORIGIN"
            BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()