# the sokol implementations library 
fips_begin_lib(sokol)
    # target_compile_options(4) 
    fips_files(sokol.c)
    fips_deps(cimgui)
    if (FIPS_EMSCRIPTEN)
        fips_libs(X11 GL m dl)
    else()
        fips_libs(X11 Xi Xcursor GL m dl asound)
    endif()
fips_end_lib()