cmake_minimum_required(VERSION 2.8)
project(cmetrics C)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Define macro to identify Windows system (without Cygwin)
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
  set(CMT_SYSTEM_WINDOWS On)
  add_definitions(-DCMT_SYSTEM_WINDOWS)
endif()

# CMetrics Version
set(CMT_VERSION_MAJOR  0)
set(CMT_VERSION_MINOR  2)
set(CMT_VERSION_PATCH  2)
set(CMT_VERSION_STR "${CMT_VERSION_MAJOR}.${CMT_VERSION_MINOR}.${CMT_VERSION_PATCH}")

# Define __FILENAME__ consistently across Operating Systems
if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__FILENAME__='\"$(subst ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")
else()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__FILENAME__=__FILE__")
endif()

# Include helpers
include(cmake/macros.cmake)
include(CheckCSourceCompiles)
include(GNUInstallDirs)

# Configuration options
option(CMT_DEV             "Enable development mode"                   No)
option(CMT_TESTS           "Enable unit testing"                       No)
option(CMT_INSTALL_TARGETS "Enable subdirectory library installations" Yes)

if(CMT_DEV)
  set(CMT_TESTS   Yes)
endif()

# Include headers and dependency headers
include_directories(
  include
  lib/monkey/include
  lib/xxHash-0.8.0/
  )

# timespec_get() support
check_c_source_compiles("
  #include <time.h>
  int main() {
     struct tm tm;
     return timespec_get(&tm, TIME_UTC);
  }" CMT_HAVE_TIMESPEC_GET)
if(CMT_HAVE_TIMESPEC_GET)
  CMT_DEFINITION(CMT_HAVE_TIMESPEC_GET)
endif()

# gmtime_r() support
check_c_source_compiles("
  #include <time.h>
  int main() {
     struct tm tm;
     struct timespec tms;
     return gmtime_r(&tms.tv_sec, &tm);
  }" CMT_HAVE_GMTIME_R)
if(CMT_HAVE_GMTIME_R)
  CMT_DEFINITION(CMT_HAVE_GMTIME_R)
endif()

# gmtime_s() support
check_c_source_compiles("
  #include <time.h>
  int main() {
     struct tm tm;
     struct timespec tms;
     return gmtime_s(&tm, &tms.tv_sec);
  }" CMT_HAVE_GMTIME_S)
if(CMT_HAVE_GMTIME_S)
  CMT_DEFINITION(CMT_HAVE_GMTIME_S)
endif()

# clock_get_time() support for macOS.
check_c_source_compiles("
  #include <mach/clock.h>
  #include <mach/mach.h>
  int main() {
      clock_serv_t cclock;
      mach_timespec_t mts;
      host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
      clock_get_time(cclock, &mts);
      return mach_port_deallocate(mach_task_self(), cclock);
  }" CMT_HAVE_CLOCK_GET_TIME)
if(CMT_HAVE_CLOCK_GET_TIME)
  CMT_DEFINITION(CMT_HAVE_CLOCK_GET_TIME)
endif()

# FIXME: MessagePack support
check_c_source_compiles("
  #include \"../../../lib/msgpack-c/include/msgpack.h\"
  int main() {
     msgpack_packer pck;
     msgpack_sbuffer sbuf;
     return 0;
  }" CMT_HAVE_MSGPACK)

configure_file(
  "${PROJECT_SOURCE_DIR}/include/cmetrics/cmt_info.h.in"
  "${PROJECT_SOURCE_DIR}/include/cmetrics/cmt_info.h"
  )

configure_file(
  "${PROJECT_SOURCE_DIR}/include/cmetrics/cmt_version.h.in"
  "${PROJECT_SOURCE_DIR}/include/cmetrics/cmt_version.h"
  )

# Installation Directories
# ========================
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(CMT_INSTALL_LIBDIR "lib")
  set(CMT_INSTALL_INCLUDEDIR "include")
else()
  set(CMT_INSTALL_LIBDIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
  set(CMT_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/include")
endif()

# xxHash
if(NOT TARGET xxhash)
  # Do something when target found
  set(XXHASH_BUILD_ENABLE_INLINE_API OFF)
  set(XXHASH_BUILD_XXHSUM OFF)
  set(BUILD_SHARED_LIBS OFF)
  add_subdirectory(lib/xxHash-0.8.0/cmake_unofficial EXCLUDE_FROM_ALL)

  if (CMT_INSTALL_TARGETS)
    install(TARGETS xxhash
      RUNTIME DESTINATION ${CMT_INSTALL_BINDIR}
      LIBRARY DESTINATION ${CMT_INSTALL_LIBDIR}
      ARCHIVE DESTINATION ${CMT_INSTALL_LIBDIR}
      COMPONENT library)
  endif()
endif()

# mpack
if(NOT TARGET mpack-static)
  include_directories(lib/mpack/src/)
  add_subdirectory(lib/mpack EXCLUDE_FROM_ALL)

  if (CMT_INSTALL_TARGETS)
    install(TARGETS mpack-static
      RUNTIME DESTINATION ${CMT_INSTALL_BINDIR}
      LIBRARY DESTINATION ${CMT_INSTALL_LIBDIR}
      ARCHIVE DESTINATION ${CMT_INSTALL_LIBDIR}
      COMPONENT library)

    install(FILES lib/mpack/src/mpack/mpack.h
      DESTINATION ${CMT_INSTALL_INCLUDEDIR}/mpack
      COMPONENT headers
      PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
  endif()
endif()

# Source code
add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(lib/monkey/include)

# Tests
if(CMT_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# Installer Generation (Cpack)
# ============================

set(CPACK_PACKAGE_VERSION ${CMT_VERSION_STR})
set(CPACK_PACKAGE_NAME "cmetrics")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "Eduardo Silva <eduardo@calyptia.com>")
set(CPACK_PACKAGE_VENDOR "Calyptia")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGING_INSTALL_PREFIX "/")

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")

if(CMT_SYSTEM_WINDOWS)
  set(CPACK_GENERATOR "ZIP")

  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-win64")
  else()
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-win32")
  endif()
endif()


# Enable components
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL} binary library headers)
set(CPACK_COMPONENTS_GROUPING "ONE_PER_GROUP")

set(CPACK_COMPONENT_BINARY_GROUP "RUNTIME")
set(CPACK_COMPONENT_LIBRARY_GROUP "RUNTIME")

# Debian package setup and name sanitizer
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

find_program(DPKG_PROGRAM dpkg DOC "dpkg program of Debian-based systems")
if(DPKG_PROGRAM)
  execute_process(
    COMMAND ${DPKG_PROGRAM} --print-architecture
    OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

  set(CPACK_DEBIAN_HEADERS_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}-headers.deb")
  set(CPACK_DEBIAN_RUNTIME_PACKAGE_NAME "${CPACK_PACKAGE_NAME}")
  set(CPACK_DEBIAN_RUNTIME_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}.deb")
  set(CPACK_DEBIAN_RUNTIME_PACKAGE_CONTROL_EXTRA
    ${CMAKE_CURRENT_SOURCE_DIR}/debian/conffiles
    )
endif()

# RPM Generation information
set(CPACK_RPM_PACKAGE_GROUP "System Environment/Daemons")
set(CPACK_RPM_PACKAGE_LICENSE "Apache v2.0")
set(CPACK_RPM_PACKAGE_RELEASE ${CPACK_PACKAGE_RELEASE})
set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/cpack/description")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Fast data collector for Linux")
set(CPACK_RPM_SPEC_MORE_DEFINE "%define ignore \#")
set(CPACK_RPM_USER_FILELIST
  "%ignore /lib"
  "%ignore /lib64"
  "%ignore /lib64/pkgconfig"
  "%ignore /usr/local"
  "%ignore /usr/local/bin")

set(CPACK_RPM_PACKAGE_AUTOREQ ON)
set(CPACK_RPM_RUNTIME_PACKAGE_NAME "${CPACK_PACKAGE_NAME}")
set(CPACK_RPM_HEADERS_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}-headers.rpm")
set(CPACK_RPM_RUNTIME_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}.rpm")

# CPack: DEB
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

# CPack: Windows System
if(CPACK_GENERATOR MATCHES "ZIP")
  set(CPACK_MONOLITHIC_INSTALL 1)
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "cmetrics")
endif()

include(CPack)
