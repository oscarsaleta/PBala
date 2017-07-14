cmake_minimum_required (VERSION 2.6)

project (PBala)

set (PBala_VERSION_MAJOR 6)
set (PBala_VERSION_MINOR 0)
set (PBala_VERSION_PATCH 2)

set(CMAKE_C_FLAGS "-O3 -Wall -Wno-unused-result")

add_subdirectory (src)


# package creation
include (InstallRequiredSystemLibraries)
set (CPACK_GENERATOR "DEB;RPM;TGZ;ZIP")
# general settings
set (CPACK_PACKAGE_CONTACT "Oscar Saleta")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "PVM SPMD job distributer")
set (CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set (CPACK_PACKAGE_VERSION_MAJOR "${PBala_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${PBala_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${PBala_VERSION_PATCH}")
# deb installer
set (CPACK_DEBIAN_PACKAGE_MAINTAINER "Oscar Saleta <osr@mat.uab.cat>")
set (CPACK_DEBIAN_PACKAGE_DEPENDS "pvm (>= 3.4.5-12)")
# rpm installer
set (CPACK_RPM_PACKAGE_LICENSE "LGPLv3")
set (CPACK_RPM_PACKAGE_VENDOR "Oscar Saleta")
include (CPack)
