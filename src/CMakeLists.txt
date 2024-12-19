cmake_minimum_required(VERSION 3.26.0) #31.0)

file(GLOB cppfiles *.cpp)
add_executable(agp24  ${cppfiles})

set(CUSTOM_LIBRARY_PATH ${PROJECT_SOURCE_DIR}/libs/glpk4.45)

find_library(glpk 
	if (MSVC) 
	   NAMES glpk_4_45_x64   #.lib 
        else()
	   NAMES glpk  #libglpk.so
	endif()
   HINTS "${CUSTOM_LIBRARY_PATH}")

target_link_libraries(agp24 ${glpk})
#target_compile_options(agp24 -O2)
