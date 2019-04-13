include_directories (
    ${PROJECT_SOURCE_DIR}/src
    $ENV{PVM_ROOT}/include
)
link_directories (
    ${PROJECT_SOURCE_DIR}/src
    $ENV{PVM_ROOT}/lib/LINUX64
)

configure_file (
    "${PROJECT_SOURCE_DIR}/src/PBala_config.h.in"
    "${PROJECT_BINARY_DIR}/PBala_config.h"
    )

include_directories ("${PROJECT_BINARY_DIR}")

add_library (PBala_lib PBala_lib.c)

add_executable (PBala PBala.c)
target_link_libraries (PBala pvm3 PBala_lib m)

add_executable (PBala_task PBala_task.c)
target_link_libraries (PBala_task pvm3 PBala_lib m)

install (TARGETS PBala PBala_task DESTINATION bin)
install (FILES "${PROJECT_BINARY_DIR}/PBala_config.h" DESTINATION include)
