cmake_minimum_required (VERSION 2.6)

project (ldjam_hiscore)

set(CMAKE_BUILD_TYPE Debug)

add_executable(simpletest ldjam_hiscore.c simpletest.c)

# TODO: Only add this if raylib can be found
add_subdirectory( samplegame )