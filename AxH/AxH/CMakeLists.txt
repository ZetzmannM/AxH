# CMakeList.txt : CMake project for AxH, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (AxH WIN32 "AxH.cpp" "AxH.h"  "math.h" "dtypes.h"  "errhndl.h" "errhndl.cpp" "env.h" "env.cpp" "utils.cpp" "utils.h" "inc_settings.h" "misc.h" "graph.h" "input.h" "input.cpp" "surface.h" "surface.cpp" "ptr.h")
set_property(TARGET AxH PROPERTY CXX_STANDARD 17)

# TODO: Add tests and install targets if needed.
FILE(GLOB LOCAL_SOURCE
    "*.hpp"
    "*.h"
)
FILE(GLOB LOCAL_SOURCE_C
    "*.cpp"
    "*.c"
)
# Main Executable                                                     

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)


SOURCE_GROUP("1HeaderFiles" FILES ${LOCAL_SOURCE})
SOURCE_GROUP("2SourceFiles" FILES ${LOCAL_SOURCE_C})