cmake_minimum_required(VERSION 3.13)
project(mtd2_psp_port C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CORE_SOURCES
    src/core/grid.c
    src/core/tower.c
    src/core/enemy.c
    src/core/wave.c
    src/core/map_data.c
    src/core/game.c
    src/core/assets.c
    src/core/render.c
    third_party/lodepng.c
)

if (PSP)
    # ---- PSP target: included by the pspdev toolchain file below ----
    add_executable(mtd2 ${CORE_SOURCES} src/platform/psp/platform_psp.c src/platform/psp/main.c)
    target_include_directories(mtd2 PRIVATE src/core third_party)

    target_link_libraries(mtd2
        pspgu pspgum
        pspge
        pspaudio pspaudiolib
        pspctrl
        pspdisplay
        m
    )

    create_pbp_file(TARGET mtd2
        ICON_PATH NULL
        BACKGROUND_PATH NULL
        PREVIEW_PATH NULL
        TITLE "MC Tower Defense 2"
        VERSION 01.00
    )
else()
    # ---- Desktop target (Linux/macOS/Windows via SDL2) ----
    find_package(SDL2 REQUIRED)

    add_executable(mtd2_desktop ${CORE_SOURCES}
        src/platform/desktop/platform_desktop.c
        src/platform/desktop/main.c)

    # SDL2's own package config (used by the official Windows devel
    # package) exposes an imported target; older distro-provided
    # find-modules (typical on Linux) set plain variables instead -
    # support whichever one was actually found.
    if (TARGET SDL2::SDL2)
        target_link_libraries(mtd2_desktop SDL2::SDL2)
    else()
        target_include_directories(mtd2_desktop PRIVATE ${SDL2_INCLUDE_DIRS})
        target_link_libraries(mtd2_desktop ${SDL2_LIBRARIES})
    endif()
    if (NOT WIN32)
        target_link_libraries(mtd2_desktop m) # MSVC has no separate libm
    endif()
    target_include_directories(mtd2_desktop PRIVATE src/core third_party)

    # Copy (not symlink - symlinks need elevated perms on Windows)
    # assets next to the binary so it runs from the build dir as-is.
    add_custom_command(TARGET mtd2_desktop POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:mtd2_desktop>/assets
        VERBATIM)
endif()
