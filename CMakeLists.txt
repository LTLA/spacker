cmake_minimum_required(VERSION 3.14)

project(spacker
    VERSION 1.0.0
    DESCRIPTION "Pack a sparse matrix of positive integers"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(spacker INTERFACE)

target_include_directories(spacker INTERFACE include/)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif() 
endif()
