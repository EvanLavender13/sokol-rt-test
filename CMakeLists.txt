#
# project: sokol-rt-test
#
cmake_minimum_required(VERSION 3.21)
project(sokol-rt-test)
# include the fips main cmake file
get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
include("${FIPS_ROOT_DIR}/cmake/fips.cmake")

# include_directories(src)
fips_setup()

set(sokol_backend SOKOL_GLCORE33)
set(slang "glsl330")

add_subdirectory(libs)
include_directories(libs)
# add_subdirectory(res)
add_subdirectory(src)

