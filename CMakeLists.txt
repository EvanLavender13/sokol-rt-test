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

project(sokol-rt-test C)
fips_ide_group("libs")
add_subdirectory(libs)
# add_subdirectory(res)
fips_ide_group("src")
add_subdirectory(src)

