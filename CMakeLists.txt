cmake_minimum_required(VERSION 3.2.0 FATAL_ERROR)

project(slog LANGUAGES C)

list (APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

find_package(Threads REQUIRED)
find_package(RT REQUIRED)

add_compile_options(-Wall)

add_library(slog STATIC src/slog.c src/slog.h)

