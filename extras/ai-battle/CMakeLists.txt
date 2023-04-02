# Copyright (C) 2005 - 2023 Settlers Freaks <sf-team at siedler25.org>
#
# SPDX-License-Identifier: GPL-2.0-or-later

add_executable(ai-battle 
    main.cpp
    AIGameManager.cpp)

target_link_libraries(ai-battle PRIVATE s25Main rttrConfig Boost::nowide Boost::program_options)
target_include_directories(ai-battle PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
