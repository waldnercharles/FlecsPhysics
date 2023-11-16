cmake_minimum_required(VERSION 3.26)
project(FlecsPhysics)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

#add_compile_options(/MT /O2 /GR- /EHa- /Oi /nologo /GL)

add_compile_definitions(_ITERATOR_DEBUG_LEVEL=0 FLECS_NDEBUG NDEBUG)

set(FLECS_SHARED OFF)
fetchcontent_declare(
        flecs
        GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
        GIT_TAG master
        GIT_PROGRESS FALSE
        SYSTEM
)
fetchcontent_makeavailable(flecs)

add_executable(FlecsPhysics main.cpp
               aabb_grid.h)
target_link_libraries(FlecsPhysics flecs_static)
