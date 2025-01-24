cmake_minimum_required(VERSION 3.2.0 FATAL_ERROR)
project(slog LANGUAGES C)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

IF (WIN32)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Od /W3")
ELSE()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O2 -Wall")
ENDIF()

add_library(slog STATIC src/slog.c src/slog.h)
add_executable(slog-sample example/example.c)

target_include_directories(slog-sample PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)
target_link_libraries(slog-sample PRIVATE slog Threads::Threads)

install(TARGETS slog DESTINATION lib)
install(FILES src/slog.h DESTINATION include)
