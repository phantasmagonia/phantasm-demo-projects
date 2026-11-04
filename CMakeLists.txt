cmake_minimum_required(VERSION 3.26)
project(PhantasmDemoProjects LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Pinned engine version. For local engine development, configure with
#   -DFETCHCONTENT_SOURCE_DIR_PHANTASM=/path/to/Phantasm
include(FetchContent)
FetchContent_Declare(
    Phantasm
    GIT_REPOSITORY https://github.com/phantasmagonia/Phantasm.git
    GIT_TAG 4679f4826341513458ac4a6c612ea680ed702ad5
)
FetchContent_MakeAvailable(Phantasm)

add_subdirectory(scavenger)
