cmake_minimum_required(VERSION 3.16.3)
project(backend C)
# Check if these libraries exist.
find_package(libwebsockets QUIET)
find_package(SQLite3 QUIET)
find_package(cJSON)
include(ExternalProject)
set(DEPS ${PROJECT_BINARY_DIR}/deps)
# don't repeatedly build ExternalProjects.
set_directory_properties(PROPERTIES EP_UPDATE_DISCONNECTED true)

# --------------------- BACKEND ---------------------
file(GLOB_RECURSE HEADERS include/*)
add_executable(backend
    test.db
    ${HEADERS}
    src/main.c
    src/database_utils.c
    src/lws.c
    src/request_handler.c
    )

target_include_directories(backend PRIVATE include ${DEPS}/include)

# --------------------- LIBCJSON ---------------------
IF(cJSON_FOUND)
    message("cJSON libraries : ${CJSON_LIBRARIES}")
    target_link_libraries(backend PUBLIC ${CJSON_LIBRARIES})
ELSE()
ExternalProject_Add(target_libcjson
    GIT_REPOSITORY  "https://github.com/DaveGamble/cJSON"
    CMAKE_ARGS      -DCMAKE_INSTALL_PREFIX:PATH=${DEPS}
    )
add_dependencies(backend target_libcjson)
add_library(libcjson SHARED IMPORTED)
set_property(TARGET libcjson PROPERTY IMPORTED_LOCATION ${DEPS}/lib/libcjson.so)
target_link_libraries(backend PUBLIC libcjson)
ENDIF()

# --------------------- LIBSQLITE ---------------------
IF(SQLite3_FOUND)
    message("SQLite3 libraries : ${SQLite3_LIBRARIES}")
    target_link_libraries(backend PUBLIC ${SQLite3_LIBRARIES})
ELSE()
    message("Will build sqlite from source...")
    ExternalProject_Add(target_sqlite3
        URL "https://sqlite.org/2021/sqlite-autoconf-3360000.tar.gz"
        CONFIGURE_COMMAND "${PROJECT_BINARY_DIR}/target_sqlite3-prefix/src/target_sqlite3/configure" "--prefix=${DEPS}"
        )
    add_library(libsqlite3 SHARED IMPORTED)
    set_property(TARGET libsqlite3 PROPERTY IMPORTED_LOCATION ${DEPS}/lib/libsqlite3.so)
    add_dependencies(backend target_sqlite3)
ENDIF()

# --------------------- LIBWEBSOCKETS ---------------------
IF(libwebsockets_FOUND)
    message("Libwebsockets libraries : ${LIBWEBSOCKETS_LIBRARIES}")
    target_link_libraries(backend PUBLIC ${LIBWEBSOCKETS_LIBRARIES})
ELSE()
    message("Will build libwebsockets from source...")
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
    add_library(libwebsockets SHARED IMPORTED)
    set_property(TARGET libwebsockets PROPERTY IMPORTED_LOCATION ${DEPS}/lib/libwebsockets.so)
ENDIF()



# Debug database
add_custom_command(
    OUTPUT test.db
    DEPENDS ${CMAKE_SOURCE_DIR}/scripts/regenerate_db.sh ${CMAKE_SOURCE_DIR}/scripts/test_db.sql
    COMMAND /bin/sh -c 'cp -r ${CMAKE_SOURCE_DIR}/scripts . && ./scripts/regenerate_db.sh'
    )

