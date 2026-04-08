cmake_minimum_required(VERSION 3.16)

project(tinygl LANGUAGES C)

enable_testing()
include(CTest)

set(TINYGL_PUBLIC_HEADERS
    include/tinygl.h
)

set(TINYGL_PRIVATE_HEADERS
    source/internal/glstd.h
    source/internal/gllist.h
    source/internal/glmain.h
    source/internal/glraster.h
    source/internal/glvector.h
)

set(TINYGL_SOURCES
    source/api/glmain.c
    source/container/gllist.c
    source/core/glraster.c
    source/core/glutil.c
    source/math/glvector.c
)

set(TINYGL_TEST_SOURCES
    tests/smoke/portable_core_smoke.c
)

set(TINYGL_DEMO_COMMON_SOURCES
    demo/common/tinygl_demo.c
)

add_library(tinygl_shared SHARED
    ${TINYGL_PUBLIC_HEADERS}
    ${TINYGL_PRIVATE_HEADERS}
    ${TINYGL_SOURCES}
)

add_library(tinygl_static STATIC
    ${TINYGL_PUBLIC_HEADERS}
    ${TINYGL_PRIVATE_HEADERS}
    ${TINYGL_SOURCES}
)

set_target_properties(tinygl_shared PROPERTIES
    OUTPUT_NAME tinygl
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN OFF
    VERSION 0.1.0
    SOVERSION 0
)

set_target_properties(tinygl_static PROPERTIES
    OUTPUT_NAME tinygl
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

target_compile_definitions(tinygl_shared PRIVATE
    TINYGL_SHARED
    TINYGL_BUILD
)

target_include_directories(tinygl_shared
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source/internal
)

target_include_directories(tinygl_static
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source/internal
)

target_link_libraries(tinygl_shared PRIVATE m)
target_link_libraries(tinygl_static PRIVATE m)

add_executable(tinygl_portable_smoke
    ${TINYGL_TEST_SOURCES}
)

set_target_properties(tinygl_portable_smoke PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)

target_include_directories(tinygl_portable_smoke PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(tinygl_portable_smoke PRIVATE
    tinygl_static
    m
)

add_test(NAME tinygl_portable_smoke
    COMMAND tinygl_portable_smoke
)

option(TINYGL_BUILD_SDL2_DEMO "Build the SDL2 TinyGL demo" ON)

if(TINYGL_BUILD_SDL2_DEMO)
    find_package(PkgConfig QUIET)

    if(PKG_CONFIG_FOUND)
        pkg_check_modules(SDL2 QUIET IMPORTED_TARGET sdl2)
    endif()

    if(SDL2_FOUND)
        add_executable(tinygl_demo_sdl2
            ${TINYGL_DEMO_COMMON_SOURCES}
            demo/common/tinygl_demo.h
            demo/platform/sdl2/tinygl_demo_sdl2.c
        )

        set_target_properties(tinygl_demo_sdl2 PROPERTIES
            C_STANDARD 99
            C_STANDARD_REQUIRED ON
        )

        target_include_directories(tinygl_demo_sdl2 PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/demo/common
        )

        target_link_libraries(tinygl_demo_sdl2 PRIVATE
            tinygl_static
            PkgConfig::SDL2
            m
        )

        add_test(NAME tinygl_demo_sdl2_smoke
            COMMAND tinygl_demo_sdl2 --frames 3
        )

        set_tests_properties(tinygl_demo_sdl2_smoke PROPERTIES
            ENVIRONMENT "SDL_VIDEODRIVER=dummy"
        )
    else()
        message(STATUS "SDL2 not found. tinygl_demo_sdl2 will not be built.")
    endif()
endif()

install(TARGETS tinygl_shared tinygl_static
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${TINYGL_PUBLIC_HEADERS}
    DESTINATION include
)
