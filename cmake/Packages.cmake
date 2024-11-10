#find_package_esa()
#find_package_esl()
#find_package_common4esl()
#find_package_logbook4esl()
#find_package_zsystem4esl()
#find_package_opengtx4esl()
#find_package_curl4esl()
#find_package_mhd4esl()
#find_package_sqlite4esl()
#find_package_unixODBC4esl()

#find_package_TinyXML2()
#find_package_logbook()
#find_package_zsystem()
#find_package_GnuTLS()
#find_package_CURL()
#find_package_libmicrohttpd()
#find_package_SQLite3()
#find_package_ODBC()

include(FetchContent)
#include(FindPkgConfig)

if(ESL_DEPENDENCIES_USE_CONAN)
    message(STATUS "Using Conan")
    include(${CMAKE_BINARY_DIR}/conan/conan_toolchain.cmake)
endif()

if(ESL_DEPENDENCIES_USE_VCPKG)
    message(STATUS "Using VCPKG")
    if(WIN32)
        set(USER_HOME_DIRECTORY $ENV{USERPROFILE})
    else()
        set(USER_HOME_DIRECTORY $ENV{HOME})
    endif()
    message(STATUS "User Home Directory: ${USER_HOME_DIRECTORY}")
    include(${USER_HOME_DIRECTORY}/opt/vcpkg/scripts/buildsystems/vcpkg.cmake)
endif()

if(ESL_DEPENDENCIES_USE_PKGCONFIG)
    find_package(PkgConfig QUIET)
endif()

function(find_custom_package NAME URL TAG)
    # Default, try 'find_package'. VCPKG or Conan may be used, if enabled
    if(NOT ${NAME}_FOUND)
        message(STATUS "Try to find ${NAME} by find_package")
        find_package(${NAME} QUIET)
        if(${NAME}_FOUND)
            message(STATUS "${NAME} has been found by using find_package")
        endif()
    endif()

    if(NOT ${NAME}_FOUND)
        message(STATUS "Try to find ${NAME} by FetchContent")
        set(COMPILE_UNITTESTS OFF)
        FetchContent_Declare(
            ${NAME}
            GIT_REPOSITORY ${URL}
            GIT_TAG ${TAG}
            GIT_SHALLOW TRUE
            OVERRIDE_FIND_PACKAGE # 'find_package(...)' will call 'FetchContent_MakeAvailable(...)'
        )
        find_package(${NAME} QUIET)
        if(${NAME}_FOUND)
            message(STATUS "${NAME} has been found by using FetchContent")
        endif()
    endif()

    if(NOT ${NAME}_FOUND)
        message(FATAL_ERROR "${NAME} NOT found")
    endif()
endfunction()

function(find_package_esa)
    find_custom_package(esa https://github.com/SLukasDE/esa master)
endfunction()

function(find_package_esl)
    find_custom_package(esl https://github.com/SLukasDE/esl master)
endfunction()

function(find_package_common4esl)
    find_custom_package(common4esl https://github.com/SLukasDE/common4esl main)
endfunction()

function(find_package_logbook)
    find_custom_package(logbook https://github.com/SLukasDE/logbook master)
endfunction()

function(find_package_logbook4esl)
    find_custom_package(logbook4esl https://github.com/SLukasDE/logbook4esl master)
endfunction()

function(find_package_zsystem)
    find_custom_package(zsystem https://github.com/SLukasDE/zsystem master)
endfunction()

function(find_package_zsystem4esl)
    find_custom_package(zsystem4esl https://github.com/SLukasDE/zsystem4esl master)
endfunction()

function(find_package_opengtx4esl)
    find_custom_package(opengtx4esl https://github.com/SLukasDE/opengtx4esl main)
endfunction()

function(find_package_curl4esl)
    find_custom_package(curl4esl https://github.com/SLukasDE/curl4esl master)
endfunction()

function(find_package_mhd4esl)
    find_custom_package(mhd4esl https://github.com/SLukasDE/mhd4esl master)
endfunction()

function(find_package_sqlite4esl)
    find_custom_package(sqlite4esl https://github.com/SLukasDE/sqlite4esl main)
endfunction()

function(find_package_odbc4esl)
    find_custom_package(odbc4esl https://github.com/SLukasDE/odbc4esl main)
endfunction()

function(find_package_TinyXML2)
    # Default, try 'find_package'. VCPKG or Conan may be used, if enabled
    if(NOT tinyxml2_FOUND)
        message(STATUS "Try to find TinyXML2 by find_package")
        find_package(tinyxml2 QUIET)
        if(tinyxml2_FOUND)
            message(STATUS "TinyXML2 has been found by using find_package")
        endif()
    endif()
	
    if(NOT tinyxml2_FOUND)
        message(STATUS "Try to find TinyXML2 by FetchContent")
        #set(tinyxml2_SHARED_LIBS ON CACHE INTERNAL "Build SHARED libraries")
        FetchContent_Declare(
            tinyxml2
            GIT_REPOSITORY https://github.com/leethomason/TinyXML2
            GIT_TAG 10.0.0
            GIT_SHALLOW TRUE
            OVERRIDE_FIND_PACKAGE # 'find_package(...)' will call 'FetchContent_MakeAvailable(...)'
        )
        find_package(tinyxml2 QUIET)
        if(tinyxml2_FOUND)
            message(STATUS "TinyXML2 has been found by using FetchContent")
        endif()
    endif()
	
    if(NOT tinyxml2_FOUND)
        message(FATAL_ERROR "TinyXML2 NOT found")
    endif()
	
	if(NOT TARGET tinyxml2::tinyxml2)
        message(FATAL_ERROR "TARGET tinyxml2::tinyxml2 does not exists")
    endif()
endfunction()

function(find_package_GnuTLS) # GnuTLS::GnuTLS
    # Default, try 'find_package'. VCPKG or Conan is used, if enabled
    if(NOT GNUTLS_FOUND)
        message(STATUS "Try to find GnuTLS by find_package")
        find_package(GnuTLS QUIET)
        if(GNUTLS_FOUND)
            message(STATUS "GnuTLS has been found by using find_package")
        endif()
    endif()

    # If still not found, try to download the sourcecode and compile it or use the prebuild binaries from the original website
    if(NOT GNUTLS_FOUND)
        if(WIN32)
            message(STATUS "Try to fetch prebuild binaries of GnuTLS from original website")
            if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/gnutls-3.7.5-w64.zip")
                file(
                    DOWNLOAD "https://www.gnupg.org/ftp/gcrypt/gnutls/v3.7/gnutls-3.7.5-w64.zip" "${CMAKE_BINARY_DIR}/external/gnutls-3.7.5-w64.zip"
                    EXPECTED_HASH MD5=d8bbbc6e0d168f0274eabba934e283cc
                    #SHOW_PROGRESS
                    )
            endif()
            if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/gnutls-3.7.5-w64")
                execute_process(
                    COMMAND $ENV{ProgramFiles}/7-Zip/7z x "${CMAKE_BINARY_DIR}/external/gnutls-3.7.5-w64.zip" "-o${CMAKE_BINARY_DIR}/external/*"
                )
            endif()
            if(EXISTS "${CMAKE_BINARY_DIR}/external/gnutls-3.7.5-w64/win64-build/lib/includes")
                file(RENAME ${CMAKE_BINARY_DIR}/external/gnutls-3.7.5-w64/win64-build/lib/includes ${CMAKE_BINARY_DIR}/external/gnutls-3.7.5-w64/win64-build/lib/gnutls)
            endif()
            if(EXISTS "${CMAKE_BINARY_DIR}/external/gnutls-3.7.5-w64")
                add_library(GnuTLS::GnuTLS UNKNOWN IMPORTED)
                set_target_properties(GnuTLS::GnuTLS PROPERTIES
                    IMPORTED_LOCATION             "${CMAKE_BINARY_DIR}/external/gnutls-3.7.5-w64/win64-build/bin/libgnutls-30.dll"
                    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}/external/gnutls-3.7.5-w64/win64-build/lib")
                set(GNUTLS_FOUND TRUE)
            endif()

            if(GNUTLS_FOUND)
                message(STATUS "GnuTLS has been found by fetiching prebuild binaries from original website")
            endif()
        else()
            message(STATUS "Try to fetch source code of GnuTLS from original website")
            if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/gnutls-3.7.5.tar.xz")
                file(
                    DOWNLOAD "https://www.gnupg.org/ftp/gcrypt/gnutls/v3.7/gnutls-3.7.5.tar.xz" "${CMAKE_BINARY_DIR}/external/gnutls-3.7.5.tar.xz"
                    EXPECTED_HASH MD5=749eb6f0e5646b90dd00521b7853b7c7
                    #SHOW_PROGRESS
                    )
            endif()
            if(EXISTS "${CMAKE_BINARY_DIR}/external/gnutls-3.7.5.tar.xz")
                file(ARCHIVE_EXTRACT INPUT "${CMAKE_BINARY_DIR}/external/gnutls-3.7.5.tar.xz" DESTINATION "${CMAKE_BINARY_DIR}/external")
            endif()
            if(EXISTS "${CMAKE_BINARY_DIR}/external/gnutls-3.7.5")
                execute_process(
                    COMMAND "${CMAKE_BINARY_DIR}/external/gnutls-3.7.5/configure" --prefix=${CMAKE_BINARY_DIR}/external/gnutls
                    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/external/gnutls-3.7.5"
                )
                execute_process(
                    COMMAND make
                    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/external/gnutls-3.7.5"
                )
                execute_process(
                    COMMAND make install
                    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/external/gnutls-3.7.5"
                )
            endif()
            if(EXISTS "${CMAKE_BINARY_DIR}/external/gnutls")
                add_library(GnuTLS::GnuTLS UNKNOWN IMPORTED)
                set_target_properties(GnuTLS::GnuTLS PROPERTIES
                    IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/external/gnutls/lib/libgnutls.so.30.33.0"
                    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}/external/gnutls/include")
                set(GNUTLS_FOUND TRUE)
            endif()

            if(GNUTLS_FOUND)
                message(STATUS "GnuTLS has been found by fetiching the source code from original website")
            endif()
        endif()
    endif()

    if(NOT GNUTLS_FOUND)
        message(FATAL_ERROR "GnuTLS NOT found")
    endif()
endfunction()

function(find_package_CURL) # CURL::libcurl
    if(BUILD_SHARED_LIBS)
        set(CURL_USE_STATIC_LIBS FALSE)
    else(BUILD_SHARED_LIBS)
        set(CURL_USE_STATIC_LIBS TRUE)
    endif(BUILD_SHARED_LIBS)
	
    # Default, try 'find_package'. VCPKG or Conan may be used, if enabled
    if(NOT CURL_FOUND)
        message(STATUS "Try to find CURL by find_package")
        find_package(CURL QUIET)
        if(CURL_FOUND)
            message(STATUS "CURL has been found by using find_package")
        endif()
    endif()
	
    if(NOT CURL_FOUND)
        message(STATUS "Try to find CURL by FetchContent")
        
        # https://github.com/curl/curl/blob/master/lib/curl_config.h.cmake
        set(BUILD_CURL_EXE      OFF)
        # Only FILE and HTTP(S) support required
        set(CURL_ENABLE_SSL     TRUE)
        set(CURL_DISABLE_FTP    TRUE)
        set(CURL_DISABLE_LDAP   TRUE)
        set(CURL_DISABLE_POP3   TRUE)
        set(CURL_DISABLE_IMAP   TRUE)
        set(CURL_DISABLE_SMB    TRUE)
        set(CURL_DISABLE_SMTP   TRUE)
        set(CURL_DISABLE_RTSP   TRUE)
        set(CURL_DISABLE_MQTT   TRUE)
        set(CURL_DISABLE_TELNET TRUE)
        set(CURL_DISABLE_TFTP   TRUE)
        set(CURL_DISABLE_DICT   TRUE)
        set(CURL_DISABLE_GOPHER TRUE)

		#[[
        if(WIN32)
            add_definitions("-DCURL_STATICLIB")
            set(ENABLE_UNICODE ON CACHE BOOL INTERNAL "Set to ON to use the Unicode version of the Windows API functions")
        endif()
        ]]

        #[[ 
        FetchContent_Declare(curl
            GIT_REPOSITORY https://github.com/curl/curl.git
            GIT_TAG  curl-8_5_0
            GIT_SHALLOW TRUE
            OVERRIDE_FIND_PACKAGE # 'find_package(...)' will call 'FetchContent_MakeAvailable(...)'
            #SUBBUILD_DIR "${DEPS_ARTIFACTS}/build/curl/"
            #TMP_DIR "${DEPS_ARTIFACTS}/tmp/curl/"
            #STAMP_DIR "${DEPS_ARTIFACTS}/stamp/curl/"
            #LOG_DIR "${DEPS_ARTIFACTS}/log/curl/"
            #BINARY_DIR "${DEPS_ARTIFACTS}/deps_builds/curl"
        )
        ]]

        FetchContent_Declare(
            curl
            URL https://curl.se/download/curl-8.5.0.tar.gz
            DOWNLOAD_EXTRACT_TIMESTAMP true
            OVERRIDE_FIND_PACKAGE # 'find_package(...)' will call 'FetchContent_MakeAvailable(...)'
        )
        
        find_package(CURL QUIET)
        if(CURL_FOUND)
            message(STATUS "CURL has been found by using FetchContent")
        endif()
    endif()
	
    
    if(NOT CURL_FOUND)
        message(FATAL_ERROR "CURL not found")
    endif()

    if(NOT TARGET CURL::libcurl)
        message(FATAL_ERROR "TARGET CURL::libcurl does not exists")
    endif()
endfunction()


function(find_package_libmicrohttpd)
    # Default, try 'find_package'. VCPKG or Conan may be used, if enabled
    if(NOT libmicrohttpd_FOUND)
        message(STATUS "Try to find libmicrohttpd by find_package")
        find_package(libmicrohttpd QUIET)
        if(libmicrohttpd_FOUND)
            message(STATUS "libmicrohttpd has been found by using find_package")
        endif()
    endif()

    # If still not found, try to download the sourcecode and compile it or use the prebuild binaries from the original website
    if(NOT libmicrohttpd_FOUND)
        if(WIN32)
            message(STATUS "Try to fetch prebuild binaries of libmicrohttpd from original website")
            if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/libmicrohttpd-0.9.77-w32-bin.zip")
                file(
                    DOWNLOAD "https://ftp.fau.de/gnu/libmicrohttpd/libmicrohttpd-0.9.77-w32-bin.zip" "${CMAKE_BINARY_DIR}/external/libmicrohttpd-0.9.77-w32-bin.zip"
                    EXPECTED_HASH MD5=100379141a825e3258e276069f2bb542
                    #SHOW_PROGRESS
                    )
            endif()
            if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/libmicrohttpd-0.9.77-w32-bin")
                execute_process(
                    COMMAND $ENV{ProgramFiles}/7-Zip/7z x "${CMAKE_BINARY_DIR}/external/libmicrohttpd-0.9.77-w32-bin.zip" "-o${CMAKE_BINARY_DIR}/external/*"
                )
            endif()
            if(EXISTS "${CMAKE_BINARY_DIR}/external/libmicrohttpd-0.9.77-w32-bin")
                add_library(libmicrohttpd::libmicrohttpd UNKNOWN IMPORTED)
                set_target_properties(libmicrohttpd::libmicrohttpd PROPERTIES
                    IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/external/libmicrohttpd-0.9.77-w32-bin/libmicrohttpd-0.9.77-w32-bin/x86_64/MinGW/shared/mingw64/bin/libmicrohttpd-12.dll"
                    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}/external/libmicrohttpd-0.9.77-w32-bin/libmicrohttpd-0.9.77-w32-bin/x86_64/MinGW/shared/mingw64/include")
                set(libmicrohttpd_FOUND TRUE)
            endif()
        else()
            message(STATUS "Try to fetch source code of libmicrohttpd from original website")
            if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/libmicrohttpd-0.9.77.tar.gz")
                file(
                    DOWNLOAD "https://ftp.fau.de/gnu/libmicrohttpd/libmicrohttpd-0.9.77.tar.gz" "${CMAKE_BINARY_DIR}/external/libmicrohttpd-0.9.77.tar.gz"
                    EXPECTED_HASH MD5=bc1b407093459ff5e7af2e3c0634d220
                    #SHOW_PROGRESS
                    )
            endif()
            if(EXISTS "${CMAKE_BINARY_DIR}/external/libmicrohttpd-0.9.77.tar.gz")
                file(ARCHIVE_EXTRACT INPUT "${CMAKE_BINARY_DIR}/external/libmicrohttpd-0.9.77.tar.gz" DESTINATION "${CMAKE_BINARY_DIR}/external")
                execute_process(
                    COMMAND "${CMAKE_BINARY_DIR}/external/libmicrohttpd-0.9.77/configure" --prefix=${CMAKE_BINARY_DIR}/external/libmicrohttpd
                    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/external/libmicrohttpd-0.9.77"
                )
                execute_process(
                    COMMAND make
                    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/external/libmicrohttpd-0.9.77"
                )
                execute_process(
                    COMMAND make install
                    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/external/libmicrohttpd-0.9.77"
                )
            endif()
            if(EXISTS "${CMAKE_BINARY_DIR}/external/libmicrohttpd")
                add_library(libmicrohttpd::libmicrohttpd UNKNOWN IMPORTED)
                set_target_properties(libmicrohttpd::libmicrohttpd PROPERTIES
                    IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/external/libmicrohttpd/lib/libmicrohttpd.so.12.61.0"
                    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}/external/libmicrohttpd/include")
                set(libmicrohttpd_FOUND TRUE)
            endif()
        endif()
    endif()

	
    if(libmicrohttpd_FOUND)
        message(STATUS "libmicrohttpd found")
    else()
        message(SEND_ERROR  "libmicrohttpd NOT found")
    endif()
    if(TARGET libmicrohttpd::libmicrohttpd)
        message(STATUS "TARGET libmicrohttpd::libmicrohttpd exists")
    else()
        message(FATAL_ERROR "TARGET libmicrohttpd::libmicrohttpd does not exists")
    endif()
endfunction()


function(find_package_SQLite3) # SQLite::SQLite3
    # Default, try 'find_package'. VCPKG or Conan may be used, if enabled
    if(NOT SQLite3_FOUND)
        message(STATUS "Try to find SQLite3 by find_package")
        find_package(SQLite3 QUIET)
        if(SQLite3_FOUND)
            message(STATUS "SQLite3 has been found by using find_package")
        endif()
    endif()
	
    
    if(NOT SQLite3_FOUND)
        message(STATUS "Try to find SQLite3 by FetchContent")
        FetchContent_Declare(
            SQLite3
            SOURCE_DIR "${CMAKE_SOURCE_DIR}/cmake/sqlite3-cmake"
            OVERRIDE_FIND_PACKAGE # 'find_package(...)' will call 'FetchContent_MakeAvailable(...)'
        )
        find_package(SQLite3 QUIET)
        if(SQLite3_FOUND)
            message(STATUS "SQLite3 has been found by using FetchContent")
        endif()
    endif()

    if(NOT SQLite3_FOUND)
        message(FATAL_ERROR "SQLite3 NOT found")
    endif()
endfunction()


function(find_package_ODBC) # ODBC::ODBC
    # Default, try 'find_package'. VCPKG or Conan may be used, if enabled
    if(NOT ODBC_FOUND)
        message(STATUS "Try to find ODBC by find_package")
        find_package(ODBC QUIET)
        if(ODBC_FOUND)
            message(STATUS "ODBC has been found by using find_package")
        endif()
    endif()


    # If still not found, try to download the sourcecode and compile it or use the prebuild binaries from the original website
    if(NOT ODBC_FOUND)
        if(UNIX)
            # https://www.unixodbc.org/
            message(STATUS "Try to fetch source code of unixODBC from original website")
            if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/unixODBC-2.3.12.tar.gz")
                file(
                    DOWNLOAD "https://www.unixodbc.org/unixODBC-2.3.12.tar.gz" "${CMAKE_BINARY_DIR}/external/unixODBC-2.3.12.tar.gz"
                    EXPECTED_HASH MD5=d62167d85bcb459c200c0e4b5a63ee48
                    #SHOW_PROGRESS
                    )
            endif()
            if(EXISTS "${CMAKE_BINARY_DIR}/external/unixODBC-2.3.12.tar.gz")
                file(ARCHIVE_EXTRACT INPUT "${CMAKE_BINARY_DIR}/external/unixODBC-2.3.12.tar.gz" DESTINATION "${CMAKE_BINARY_DIR}/external")
            endif()
            if(EXISTS "${CMAKE_BINARY_DIR}/external/unixODBC-2.3.12")
                execute_process(
                    COMMAND "${CMAKE_BINARY_DIR}/external/unixODBC-2.3.12/configure" --prefix=${CMAKE_BINARY_DIR}/external/unixODBC
                    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/external/unixODBC-2.3.12"
                )
                execute_process(
                    COMMAND make
                    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/external/unixODBC-2.3.12"
                )
                execute_process(
                    COMMAND make install
                    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/external/unixODBC-2.3.12"
                )
            endif()
            if(EXISTS "${CMAKE_BINARY_DIR}/external/unixODBC")
                add_library(ODBC::ODBC UNKNOWN IMPORTED)
                set_target_properties(ODBC::ODBC PROPERTIES
                    IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/external/unixODBC/lib/libodbc.so.2.0.0"
                    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}/external/unixODBC/include")
                set(ODBC_FOUND TRUE)
            endif()

            if(ODBC_FOUND)
                message(STATUS "unixODBC has been found by fetiching the source code from original website")
            endif()
        endif(UNIX)

        if(APPLE)
            # https://github.com/openlink/iODBC
            message(STATUS "Try to fetch source code of iODBC from original website")

            # ...

            if(ODBC_FOUND)
                message(STATUS "iODBC has been found by fetiching the source code from original website")
            endif()
        endif(APPLE)
    endif()

    if(NOT ODBC_FOUND)
        message(FATAL_ERROR "ODBC NOT found")
    endif()
endfunction()
