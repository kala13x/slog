cmake_minimum_required(VERSION 3.2.0 FATAL_ERROR)

project(slog LANGUAGES C)

list (APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(RT REQUIRED)

add_compile_options(-Wall)

add_library(slog STATIC src/slog.c src/slog.h)

add_executable(slog-sample example/example.c)
target_include_directories(slog-sample PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)
target_link_libraries(slog-sample PRIVATE slog Threads::Threads)
