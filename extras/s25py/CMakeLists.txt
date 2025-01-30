# Copyright (C) 2005 - 2024 Settlers Freaks <sf-team at siedler25.org>
#
# SPDX-License-Identifier: GPL-2.0-or-later

# Get python suffix
execute_process(
    COMMAND python3-config --extension-suffix
    OUTPUT_VARIABLE PYTHON3_SUFFIX
    RESULT_VARIABLE COMMAND_RESULT
    ERROR_VARIABLE COMMAND_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT COMMAND_RESULT EQUAL 0)
    message(WARNING "Command failed with error code: ${COMMAND_RESULT}")
    message(FATAL_ERROR "Error output: ${COMMAND_ERROR}")
endif()

# Get python include dir
execute_process(
    COMMAND python3-config --includes
    OUTPUT_VARIABLE PYTHON3_INCLUDES
    RESULT_VARIABLE COMMAND_RESULT
    ERROR_VARIABLE COMMAND_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Split the output into a list
string(REGEX MATCHALL "-I[^ ]+" PYTHON3_INCLUDES_LIST "${PYTHON3_INCLUDES}")

# Create a new list without the '-I' prefix
set(PYTHON3_INCLUDES)
foreach(item IN LISTS PYTHON3_INCLUDES_LIST)
    string(REPLACE "-I" "" item "${item}")
    list(APPEND PYTHON3_INCLUDES "${item}")
endforeach()

if(NOT COMMAND_RESULT EQUAL 0)
    message(WARNING "Command failed with error code: ${COMMAND_RESULT}")
    message(FATAL_ERROR "Error output: ${COMMAND_ERROR}")
endif()

add_library(s25py SHARED
    ${PROJECT_SOURCE_DIR}/libs/s25main/ai/s25py/pyBuilding.cpp
    pyGame.cpp
    s25module.cpp
    ${PROJECT_SOURCE_DIR}/libs/s25main/ai/s25py/pyPlayer.cpp
    ${PROJECT_SOURCE_DIR}/libs/s25main/ai/s25py/s25py.cpp)

set_target_properties(s25py PROPERTIES PREFIX "")
set_target_properties(s25py PROPERTIES SUFFIX "${PYTHON3_SUFFIX}")
set_target_properties(s25py PROPERTIES C_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON)

target_link_libraries(s25py PRIVATE s25Main Boost::nowide)

target_include_directories(s25py PRIVATE 
    ${PROJECT_SOURCE_DIR}/external/pybind11/include
    ${PROJECT_SOURCE_DIR}/libs/s25main/ai/s25py
    ${PYTHON3_INCLUDES})
