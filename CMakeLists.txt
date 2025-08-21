cmake_minimum_required(VERSION 3.16)
project(CoinSorter LANGUAGES C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Options
option(BUILD_SUPERFORCE "Build superforce unified binary" ON)
option(BUILD_UI "Build interactive text UI (no external deps)" ON)
option(ENABLE_DOXYGEN "Generate API documentation with Doxygen" OFF)

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(coins_core
    src/coin_algorithms.c
    src/coin_systems.c
  src/env.c
    src/beta.c
    src/casimir.c
  src/simulation.c
  src/color.c)

add_executable(coinsorter src/coinsorter.c)
target_link_libraries(coinsorter PRIVATE coins_core m)
target_compile_options(coins_core PRIVATE -Wall -Wextra -Werror)
target_compile_options(coinsorter PRIVATE -Wall -Wextra -Werror)

if(BUILD_SUPERFORCE)
  add_executable(superforce src/superforce_main.c)
  target_link_libraries(superforce PRIVATE coins_core m)
  target_compile_options(superforce PRIVATE -Wall -Wextra -Werror)
endif()

if(BUILD_UI)
  add_executable(superforce_ui src/superforce_ui.c)
  target_link_libraries(superforce_ui PRIVATE coins_core m)
  target_compile_options(superforce_ui PRIVATE -Wall -Wextra -Werror)
endif()

# Installation
include(GNUInstallDirs)
install(TARGETS coinsorter
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
if(BUILD_SUPERFORCE)
  install(TARGETS superforce RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
if(BUILD_UI)
  install(TARGETS superforce_ui RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
install(TARGETS coins_core ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
file(GLOB PUB_HEADERS ${CMAKE_SOURCE_DIR}/include/*.h)
install(FILES ${PUB_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Tests
option(BUILD_TESTS "Build test executables" ON)
if(BUILD_TESTS)
  add_executable(test_basic tests/test_basic.c)
  target_link_libraries(test_basic PRIVATE coins_core m)
  target_compile_options(test_basic PRIVATE -Wall -Wextra -Werror)
  add_test(NAME basic COMMAND test_basic)
  add_executable(test_sim tests/test_sim.c)
  target_link_libraries(test_sim PRIVATE coins_core m)
  target_compile_options(test_sim PRIVATE -Wall -Wextra -Werror)
  add_test(NAME sim COMMAND test_sim)
  include(CTest)
endif()

# Doxygen
if(ENABLE_DOXYGEN)
  find_package(Doxygen REQUIRED dot)
  set(DOXYGEN_GENERATE_HTML YES)
  set(DOXYGEN_GENERATE_LATEX NO)
  set(DOXYGEN_EXTRACT_STATIC YES)
  set(DOXYGEN_FULL_PATH_NAMES NO)
  set(DOXYGEN_PROJECT_NAME "CoinSorter")
  set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/docs)
  doxygen_add_docs(docs
      ${CMAKE_SOURCE_DIR}/include
      ${CMAKE_SOURCE_DIR}/src
      COMMENT "Generate API docs")
endif()
