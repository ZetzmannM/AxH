# CMakeList.txt : CMake project for AxH, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
add_library(glew SHARED IMPORTED) 
set_target_properties(glew PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/../lib/glew32.dll"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/../include/"
)

# Add source to this project's executable.
add_executable (AxH WIN32 "AxH.cpp" "AxH.h"  "math.h" "dtypes.h"  "errhndl.h" "errhndl.cpp" "env.h" "env.cpp" "utils.cpp" "utils.h" "inc_settings.h" "misc.h" "graph.h" "input.h" "input.cpp" "surface.h" "surface.cpp" "ptr.h"         "fileio.h" "fileio.cpp" "res_type.h"  )
set_property(TARGET AxH PROPERTY CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20")
target_link_libraries(AxH glew opengl)


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