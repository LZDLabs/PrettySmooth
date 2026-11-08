cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(PrettySmooth VERSION 1.1.0)

# Todos os .cpp dentro de src (recursivo)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Geode SDK nao encontrado! Defina a variavel de ambiente GEODE_SDK apontando para o Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

setup_geode_mod(${PROJECT_NAME})

# Build sem o Geode CLI (GEODE_DISABLE_CLI_CALLS=ON): o setup_geode_mod
# pula a parte que define o ID e o nome do binário, então fazemos aqui.
if (GEODE_DISABLE_CLI_CALLS)
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/mod.json" PS_MOD_JSON)
    string(JSON PS_MOD_ID GET "${PS_MOD_JSON}" "id")
    target_compile_definitions(${PROJECT_NAME} PRIVATE GEODE_MOD_ID="${PS_MOD_ID}")
    set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" OUTPUT_NAME ${PS_MOD_ID})
    if (DEFINED GEODE_MOD_BINARY_SUFFIX)
        set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ${GEODE_MOD_BINARY_SUFFIX})
    endif()
endif()
