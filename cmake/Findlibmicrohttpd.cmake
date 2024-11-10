#[=======================================================================[.rst:
Findlibmicrohttpd
-----------------

Find the Micro HTTP Daemon library (libmicrohttpd)

Result Variables
^^^^^^^^^^^^^^^^

``libmicrohttpd_FOUND``
  System has libmicrohttpd
``libmicrohttpd_INCLUDE_DIR``
  The libmicrohttpd include directory
``libmicrohttpd_LIBRARIES``
  The libraries needed to use libmicrohttpd
``libmicrohttpd_VERSION``
  version of libmicrohttpd.

Copyright 2024 Sven Lukas <sven.lukas@gmail.com>

#]=======================================================================]

if (libmicrohttpd_INCLUDE_DIR AND libmicrohttpd_LIBRARY)
  # in cache already
  set(libmicrohttpd_FIND_QUIETLY TRUE)
endif ()

if (NOT WIN32)
  find_package(PkgConfig QUIET)
  pkg_check_modules(PC_LIBMHD QUIET libmicrohttpd)
endif ()

find_path(libmicrohttpd_INCLUDE_DIR
    NAMES microhttpd.h
    HINTS ${PC_LIBMHD_INCLUDEDIR} ${PC_LIBMHD_INCLUDE_DIRS})

find_library(libmicrohttpd_LIBRARY
    NAMES libmicrohttpd microhttpd
    HINTS ${PC_LIBMHD_LIBDIR} ${PC_LIBMHD_LIBRARY_DIRS})

mark_as_advanced(libmicrohttpd_INCLUDE_DIR libmicrohttpd_LIBRARY)

if (PC_LIBMHD_VERSION)
    set(libmicrohttpd_VERSION ${PC_LIBMHD_VERSION})
elseif (libmicrohttpd_INCLUDE_DIR AND EXISTS "${libmicrohttpd_INCLUDE_DIR}/microhttpd.h")
    file(STRINGS "${libmicrohttpd_INCLUDE_DIR}/microhttpd.h" version-file
        REGEX "^#define[ \t]+MHD_VERSION[ \t]+([^\"]+).*")
    string(REGEX REPLACE "^#define[ \t]+MHD_VERSION[ \t]+([^\"]+).*" "\\1" version-num "${version-file}")

    # parse version-num: '#define MHD_VERSION 0x00097700' is version "00.09.77.00"
    string(SUBSTRING ${version-num} 2 2 version-major)
    string(SUBSTRING ${version-num} 4 2 version-minor)
    string(SUBSTRING ${version-num} 6 2 version-revision)
    string(SUBSTRING ${version-num} 8 2 version-patch)
    set(libmicrohttpd_VERSION "${version-major}.${version-minor}.${version-revision}.${version-patch}" CACHE "Libmicrohttpd version")
    
    unset(version-file)
    unset(version-num)
    unset(version-major)
    unset(version-minor)
    unset(version-revision)
    unset(version-patch)
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(libmicrohttpd
    REQUIRED_VARS libmicrohttpd_LIBRARY libmicrohttpd_INCLUDE_DIR
    VERSION_VAR libmicrohttpd_VERSION)

if(libmicrohttpd_FOUND)
    set(libmicrohttpd_LIBRARIES    ${libmicrohttpd_LIBRARY})
    set(libmicrohttpd_INCLUDE_DIRS ${libmicrohttpd_INCLUDE_DIR})

    if(NOT TARGET libmicrohttpd::libmicrohttpd)
        add_library(libmicrohttpd::libmicrohttpd UNKNOWN IMPORTED)
        set_target_properties(libmicrohttpd::libmicrohttpd PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${libmicrohttpd_INCLUDE_DIRS}"
            #INTERFACE_COMPILE_DEFINITIONS "${libmicrohttpd_DEFINITIONS}"
            IMPORTED_LINK_INTERFACE_LANGUAGES "C"
            IMPORTED_LOCATION "${libmicrohttpd_LIBRARY}")
    endif()
endif()
