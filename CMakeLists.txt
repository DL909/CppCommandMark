cmake_minimum_required(VERSION 3.22)
project(CppCommandMark)
set(CMAKE_CXX_STANDARD 20)

# Set build type to Debug
set(CMAKE_BUILD_TYPE "Debug")

# Add debugging flags for C++
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")

# Add debugging flags for C
set(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -O0 -Wall -g -ggdb")

add_executable(CppCommandMark main.cpp
        Util/opt_processor.cpp
        Tasks/Task.h
        Tasks/OneLineTask.cpp
        Tasks/TaskTypeRegistrar.h
        Util/config_and_constants.h
        Util/util.cpp
        Util/util.h
)

add_executable(test test.cpp
        Util/opt_processor.cpp
        Tasks/Task.h
        Tasks/OneLineTask.cpp
        Tasks/TaskTypeRegistrar.h
        Util/config_and_constants.h
        Util/util.cpp
        Util/util.h
)

set(ENV{VCPKG_ROOT} "/Users/909dl/.local/vcpkg")
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")


set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)

target_include_directories(CppCommandMark PRIVATE ${CURSES_INCLUDE_DIRS})
target_compile_options(CppCommandMark PRIVATE ${CURSES_CFLAGS})
target_link_libraries(CppCommandMark PRIVATE ${CURSES_LIBRARIES})

target_include_directories(test PRIVATE ${CURSES_INCLUDE_DIRS})
target_compile_options(test PRIVATE ${CURSES_CFLAGS})
target_link_libraries(test PRIVATE ${CURSES_LIBRARIES})

find_package(fmt CONFIG REQUIRED)
target_link_libraries(CppCommandMark PRIVATE fmt::fmt)
target_link_libraries(test PRIVATE fmt::fmt)

add_subdirectory(3rd-party/fuzzy-match)
target_include_directories(CppCommandMark PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/fuzzy-match)
target_link_libraries(CppCommandMark PUBLIC fuzzy_match)
target_include_directories(test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/fuzzy-match)
target_link_libraries(test PUBLIC fuzzy_match)

find_package(pugixml CONFIG REQUIRED)
target_link_libraries(CppCommandMark PRIVATE pugixml::static pugixml::pugixml)
target_link_libraries(test PRIVATE pugixml::static pugixml::pugixml)











