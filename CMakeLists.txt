
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.11)
include(FetchContent)
FetchContent_Declare(
  espidflib
  GIT_REPOSITORY https://github.com/studeo14/esp-idf-lib.git
)
FetchContent_MakeAvailable(espidflib)
list(APPEND EXTRA_COMPONENT_DIRS ${espidflib_SOURCE_DIR}/components)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(blink)
