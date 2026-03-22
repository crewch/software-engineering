cmake_minimum_required(VERSION 3.12...3.31)
project(lab2 CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(DownloadUserver)

find_package(
    userver
    COMPONENTS core chaotic#
    QUIET
)
if(NOT userver_FOUND)
    # Tries TRY_DIR first, falls back to downloading userver from GitHub using CPM.
    download_userver(TRY_DIR third_party/userver)
endif()

#finding jwt-cpp
find_package(jwt-cpp QUIET)
if (NOT jwt-cpp_FOUND)
  message("Loading JWT-CPP")
  include(cmake/get_cpm.cmake)
  # download jwt-cpp
  CPMAddPackage(
    NAME jwt-cpp
    GITHUB_REPOSITORY Thalhammer/jwt-cpp
    VERSION 0.7.2
    OPTIONS "JWT_BUILD_EXAMPLES OFF" "JWT_BUILD_TESTS OFF" "JWT_CMAKE_FILES_INSTALL ON"
  )
endif()

userver_setup_environment()

# Common sources
include_directories(src)

add_library(
    ${PROJECT_NAME}_objs OBJECT
    src/greeting.cpp #
    src/auth/jwt_auth_checker.cpp #
    src/auth/jwt_auth_factory.cpp #
    src/views/hello/handler.cpp #
)
target_link_libraries(
    ${PROJECT_NAME}_objs
    PUBLIC userver::core #
    PUBLIC jwt-cpp::jwt-cpp #
)
target_include_directories(${PROJECT_NAME}_objs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/)

# /// [chaotic] cmake
file(GLOB_RECURSE SCHEMAS ${CMAKE_CURRENT_SOURCE_DIR}/docs/*.yaml)
userver_target_generate_chaotic(
    ${PROJECT_NAME}-chgen
    # Generate serializers for responses
    GENERATE_SERIALIZERS
    # Map '/components/schemas/*' JSONSchema types to C++ types in 'jwt-auth::STEM' namespace
    LAYOUT "/components/schemas/([^/]*)/=lab2::{stem}::{0}"
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/src
    SCHEMAS ${SCHEMAS}
    RELATIVE_TO ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(${PROJECT_NAME}_objs PUBLIC ${PROJECT_NAME}-chgen)
# /// [chaotic]


# The Service
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs)

# Functional testing
userver_testsuite_add_simple(REQUIREMENTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/requirements.txt)

# Install
include(GNUInstallDirs)

if(DEFINED ENV{PREFIX})
    message(STATUS "Set install prefix: $ENV{PREFIX}")
    file(TO_CMAKE_PATH "$ENV{PREFIX}" PREFIX_PATH)
    set(CMAKE_INSTALL_PREFIX "${PREFIX_PATH}")
endif()

file(GLOB CONFIGS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.yaml ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.json)

install(
    TARGETS ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT ${PROJECT_NAME}
)
install(
    FILES ${CONFIGS_FILES}
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}
    COMPONENT ${PROJECT_NAME}
)