cmake_minimum_required(VERSION 2.6)

project(PBala)

set(PBala_VERSION 4.1.1)

configure_file(
    "${PROJECT_SOURCE_DIR}/PBala_config.h.in"
    "${PROJECT_BINARY_DIR}/PBala_config.h"
    )

include_directories("${PROJECT_BINARY_DIR}")

add_library(PBala_lib PBala_lib.c)

add_executable(PBala PBala.c)
target_link_libraries(PBala pvm3 PBala_lib)

add_executable(PBala_task PBala_task.c)
target_link_libraries(PBala_task pvm3 PBala_lib)
