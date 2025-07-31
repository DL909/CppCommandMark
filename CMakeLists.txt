cmake_minimum_required(VERSION 3.22)
project(CppCommandMark)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_TOOLCHAIN_FILE /opt/vcpkg/scripts/buildsystems/vcpkg.cmake)

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)
find_package(fmt CONFIG REQUIRED)
add_subdirectory(3rd-party/fuzzy-match)

add_executable(CppCommandMark main.cpp
        CppCommandMark.cpp
        CppCommandMark.h)

target_include_directories(CppCommandMark PRIVATE ${CURSES_INCLUDE_DIRS})
target_compile_options(CppCommandMark PRIVATE ${CURSES_CFLAGS})
target_link_libraries(CppCommandMark PRIVATE ${CURSES_LIBRARIES})
target_link_libraries(CppCommandMark PRIVATE fmt::fmt)

target_include_directories(CppCommandMark PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/fuzzy-match)
target_link_libraries(CppCommandMark PUBLIC fuzzy_match)

add_executable(test test.cpp
        CppCommandMark.cpp
        CppCommandMark.h)

target_include_directories(test PRIVATE ${CURSES_INCLUDE_DIRS})
target_compile_options(test PRIVATE ${CURSES_CFLAGS})
target_link_libraries(test PRIVATE ${CURSES_LIBRARIES})

target_include_directories(test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/fuzzy-match)
target_link_libraries(test PUBLIC fuzzy_match)
