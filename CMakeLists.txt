cmake_minimum_required(VERSION 3.16.3)
project(backend C)
find_program(MAKE NAMES make)
find_package(libwebsockets QUIET)
find_package(sqlite3 QUIET)

add_executable(backend
    test.db
    ${HEADERS}
    src/main.c
    src/database_connection.c
    src/lws_protocol.c
    src/util.c
    src/lws.c
    src/lws_protocol_init.c
    src/request_handler.c)

include(ExternalProject)
# don't repeatedly build ExternalProjects.
set_directory_properties(PROPERTIES EP_UPDATE_DISCONNECTED true)
set(DEPS ${PROJECT_BINARY_DIR}/deps)

ExternalProject_Add(target_libcjson
    GIT_REPOSITORY  "https://github.com/DaveGamble/cJSON"
    CMAKE_ARGS      -DCMAKE_INSTALL_PREFIX:PATH=${DEPS}
    )
add_dependencies(backend target_libcjson)

IF(NOT sqlite3_FOUND)
ExternalProject_Add(target_sqlite3
    URL "https://sqlite.org/2021/sqlite-autoconf-3360000.tar.gz"
    CONFIGURE_COMMAND "${PROJECT_BINARY_DIR}/target_sqlite3-prefix/src/target_sqlite3/configure" "--prefix=${DEPS}"
    )
add_library(libsqlite3 SHARED IMPORTED)
set_property(TARGET libsqlite3 PROPERTY IMPORTED_LOCATION ${DEPS}/lib/libsqlite3.so)
add_dependencies(backend target_sqlite3)
ENDIF()

IF(NOT libwebsockets_FOUND)
ExternalProject_Add(target_libwebsockets
    GIT_REPOSITORY  "https://libwebsockets.org/repo/libwebsockets"
    GIT_TAG         v4.2-stable
    CMAKE_ARGS      -DCMAKE_INSTALL_PREFIX:PATH=${DEPS} -DOPENSSL_ROOT_DIR=${DEPS}/libs -DCMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE=${DEPS}/include -DLWS_WITH_HTTP2=1
    )
add_dependencies(backend target_libwebsockets)
ExternalProject_Add(target_libssl
    GIT_REPOSITORY "git://git.openssl.org/openssl.git"
    GIT_PROGRESS    TRUE
    CONFIGURE_COMMAND "${PROJECT_BINARY_DIR}/target_libssl-prefix/src/target_libssl/Configure" "--prefix=${DEPS}"
    )
ExternalProject_Add_StepDependencies(target_libwebsockets build target_libssl)
ENDIF()

add_library(libcjson SHARED IMPORTED)
set_property(TARGET libcjson PROPERTY IMPORTED_LOCATION ${DEPS}/lib64/libcjson.so)
add_library(libwebsockets SHARED IMPORTED)
set_property(TARGET libwebsockets PROPERTY IMPORTED_LOCATION ${DEPS}/lib/libwebsockets.so)
file(GLOB_RECURSE HEADERS include/*)
#backend
target_include_directories(backend PRIVATE include ${DEPS}/include)
target_link_libraries(backend PUBLIC libcjson libwebsockets libsqlite3)

# Debug database
add_custom_command(
    OUTPUT test.db
    DEPENDS ${CMAKE_SOURCE_DIR}/scripts/regenerate_db.sh ${CMAKE_SOURCE_DIR}/scripts/test_db.sql
    COMMAND /bin/sh -c 'cp -r ${CMAKE_SOURCE_DIR}/scripts . && ./scripts/regenerate_db.sh'
    )

