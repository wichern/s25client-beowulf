# Copyright (C) 2005 - 2024 Settlers Freaks <sf-team at siedler25.org>
#
# SPDX-License-Identifier: GPL-2.0-or-later

add_executable(beowulf-rl
    main.cpp
    Environment.cpp
    GameState.cpp
    HeadlessGame.cpp
    Observer.cpp
    BuildLocationSelectorTrainer.cpp)
#target_compile_definitions(beowulf-rl PRIVATE ARMA_NO_DEBUG)
#target_compile_definitions(beowulf-rl PRIVATE MLPACK_ENABLE_ANN_PRINT_DEBUG ARMA_DONT_USE_WRAPPER ARMA_EXTRA_DEBUG)
target_link_libraries(beowulf-rl PRIVATE armadillo openblas s25Main Boost::program_options Boost::nowide)

if(WIN32)
    include(GatherDll)
    gather_dll_copy(ai-battle)
endif()

set(S25RTTR_URL "https://www.siedler25.org/uploads/nightly/s25rttr_20250614-5cc1ddb3dd2a565c51b9d2e8e4cb2b310e7ebc65-linux.x86_64.tar.bz2")
set(S25RTTR_ARCHIVE "${CMAKE_BINARY_DIR}/s25rttr.tar.bz2")
set(S25RTTR_SRC_DIR "${CMAKE_BINARY_DIR}/s25rttr_20250614")
set(S25RTTR_INSTALL_DIR "${CMAKE_BINARY_DIR}/share/s25rttr/RTTR")

include(ExternalProject)
ExternalProject_Add(s25rttr_assets
    URL                 ${S25RTTR_URL}
    DOWNLOAD_DIR        ${CMAKE_BINARY_DIR}
    SOURCE_DIR          ${S25RTTR_SRC_DIR}
    CONFIGURE_COMMAND   ""
    BUILD_COMMAND       ""
    INSTALL_COMMAND     ${CMAKE_COMMAND} -E copy_directory
                        ${S25RTTR_SRC_DIR}/share/s25rttr/RTTR
                        ${S25RTTR_INSTALL_DIR}
    LOG_DOWNLOAD        ON
    LOG_INSTALL         ON
)

add_dependencies(beowulf-rl s25rttr_assets)

add_custom_command(
    TARGET beowulf-rl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/beowulf.ini
            ${CMAKE_BINARY_DIR}
    COMMENT "Copying beowulf.ini to build directory"
)
