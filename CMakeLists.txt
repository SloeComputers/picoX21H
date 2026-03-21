#-------------------------------------------------------------------------------
# Copyright (c) 2025 John D. Haughton
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.20)

project(picoX21H
        LANGUAGES CXX C ASM
        VERSION 0.03)

include(CTest)

set(HWR_CONFIG picoX21H)

add_subdirectory(Platform)
add_subdirectory(Source)
