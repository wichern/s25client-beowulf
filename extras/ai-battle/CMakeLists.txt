# Copyright (C) 2005 - 2024 Settlers Freaks <sf-team at siedler25.org>
#
# SPDX-License-Identifier: GPL-2.0-or-later

add_library(s25py SHARED s25py.cpp HeadlessGame.cpp)
set_target_properties(s25py PROPERTIES PREFIX "")
set_target_properties(s25py PROPERTIES SUFFIX ".cpython-311-x86_64-linux-gnu.so") # see python3-config --extension-suffix
target_link_libraries(s25py PRIVATE s25Main Boost::nowide)
set_property(TARGET s25py PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(s25py PROPERTIES C_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON)
target_include_directories(s25py PRIVATE 
    ${PROJECT_SOURCE_DIR}/external/pybind11/include
    /usr/include/python3.11) # see python3-config --includes

# TODO: use execute_process instead of fixed path to python3.11
# TODO: use execute_process for extension suffix

add_executable(ai-battle main.cpp HeadlessGame.cpp)
target_link_libraries(ai-battle PRIVATE s25Main Boost::program_options Boost::nowide)
