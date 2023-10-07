cmake_minimum_required(VERSION 3.5)
project(dbusparser)
set (CMAKE_CXX_STANDARD 20)
find_package(Boost 1.82.0 ) 

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS}) 
endif()
include_directories(/opt/homebrew/Cellar/nlohmann-json/3.11.2/include/)
add_executable(the_executable
    main.cpp)
