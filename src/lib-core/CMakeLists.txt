find_package(fmt CONFIG REQUIRED)

add_library(core SHARED)
add_library(mg::core ALIAS core)
target_sources(core PRIVATE
    src/core.cpp
)
target_include_directories(core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(core PUBLIC fmt::fmt)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_options(core PRIVATE
        -fPIC
        $<$<CONFIG:Debug>: -g -Wall -Wextra -Werror>
    )

    set_target_properties(core PROPERTIES
        VERSION 1.0.0
        SOVERSION 1)

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBURING liburing)

    if(LIBURING_FOUND)
        message(STATUS "liburing found")
        target_include_directories(core PRIVATE ${LIBURING_INCLUDE_DIRS})
        target_link_libraries(core PRIVATE ${LIBURING_LIBRARIES})
    else()
        message(STATUS "liburing NOT found, disabling io_uring features")
        target_compile_definitions(core PRIVATE DISABLE_IO_URING)
    endif()
endif()
