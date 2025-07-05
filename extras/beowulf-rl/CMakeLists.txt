# Copyright (C) 2005 - 2024 Settlers Freaks <sf-team at siedler25.org>
#
# SPDX-License-Identifier: GPL-2.0-or-later

add_executable(beowulf-rl
    main.cpp
    Environment.cpp
    GameState.cpp
    HeadlessGame.cpp
    Observer.cpp)
#target_compile_definitions(beowulf-rl PRIVATE ARMA_NO_DEBUG)
#target_compile_definitions(beowulf-rl PRIVATE MLPACK_ENABLE_ANN_PRINT_DEBUG ARMA_DONT_USE_WRAPPER ARMA_EXTRA_DEBUG)
target_link_libraries(beowulf-rl PRIVATE armadillo openblas s25Main Boost::program_options Boost::nowide)

if(WIN32)
    include(GatherDll)
    gather_dll_copy(ai-battle)
endif()
