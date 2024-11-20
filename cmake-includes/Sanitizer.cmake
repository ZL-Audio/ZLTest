# Enable Sanitizer
if (DEFINED ENV{SANITIZER_FLAG})
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
    message("Enable Address Sanitizer")
    if (WIN32)
        add_compile_definitions($<$<CONFIG:Debug>:_ITERATOR_DEBUG_LEVEL=2>)
    endif ()
endif ()