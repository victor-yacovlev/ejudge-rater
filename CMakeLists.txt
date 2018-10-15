project(ejudge-rater)
cmake_minimum_required(VERSION 3.5.0)

find_package(PkgConfig REQUIRED)
pkg_check_modules(HTMLCXX REQUIRED htmlcxx>=0.86)
set(CMAKE_CXX_FLAGS "-ftrapv -std=c++17")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O2 -Wall -Werror")

set(SOURCES rater.cpp)
set(TARGET ${PROJECT_NAME})

add_executable(${TARGET} ${SOURCES})
target_include_directories(${TARGET} PRIVATE ${HTMLCXX_INCLUDE_DIRS})
target_link_libraries(${TARGET} ${HTMLCXX_LIBRARIES})

install(
  TARGETS ${TARGET}
  RUNTIME DESTINATION bin
)
