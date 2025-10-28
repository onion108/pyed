cmake_minimum_required(VERSION 4.1)
project(pyed)
set(CMAKE_C_STANDARD 90)


include(FetchContent)

FetchContent_Declare(
    stonevalley
    GIT_REPOSITORY https://github.com/coshcage/StoneValley.git
)
FetchContent_MakeAvailable(stonevalley)

function(use_stonevalley target)
    file(GLOB stonevalley_SRCS LIST_DIRECTORIES false ${stonevalley_SOURCE_DIR}/src/*.c)
    target_sources(${target} PRIVATE ${stonevalley_SRCS})
    target_include_directories(${target} PRIVATE ${stonevalley_SOURCE_DIR}/src)
endfunction()

add_executable(pyed)
use_stonevalley(pyed)
add_subdirectory(src)

