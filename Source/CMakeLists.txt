#-------------------------------------------------------------------------------
# Copyright (c) 2025 John D. Haughton
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------

add_subdirectory(chip)
add_subdirectory(DX21)

add_picoX21H_executable(picoX21H
                           picoX21H.cpp
                        LIBRARIES
                           DX21)
