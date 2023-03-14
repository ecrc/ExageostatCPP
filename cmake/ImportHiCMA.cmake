
# Copyright (c) 2017-2023 King Abdullah University of Science and Technology,
# Copyright (c) 2023 by Brightskies inc,
# All rights reserved.
# ExaGeoStat is a software package, provided by King Abdullah University of Science and Technology (KAUST).

# @file ImportHiCMA.cmake
# @version 1.0.0
# @author Sameh Abdulah
# @date 2023-03-13

message("")
message("---------------------------------------- Hicma")
message(STATUS "Checking for Chameleon")
include(macros/BuildHiCMA)

if (NOT TARGET HICMA_FOUND)
    include(FindPkgConfig)
    find_package(HICMA QUIET)

    if (HICMA_FOUND)
        message("   Found HiCMA: ${HICMA_LIBDIR}")
    else ()

        set(hicma_installpath ${CMAKE_BINARY_DIR}/_deps/hicma-install)
        set(HICMA_DIR "${hicma_installpath}/")
        BuildHiCMA(HiCMA "https://github.com/ecrc/hicma.git" "v1.0.0")
        find_package(HICMA REQUIRED)
    endif ()
else()
    message("   HiCMA already included")
endif()

include_directories(${HICMA_INCLUDE_DIRS_DEP})
if (HICMA_LINKER_FLAGS)
    list(APPEND CMAKE_EXE_LINKER_FLAGS "${HICMA_LINKER_FLAGS}")
endif ()
if (HICMA_LIBRARY_DIRS)
    # The RPATH to be used when installing
    list(APPEND CMAKE_INSTALL_RPATH "${HICMA_LIBRARY_DIRS}")
    link_directories(${HICMA_LIBRARY_DIRS})
endif ()
# Insert to dependencies
if (HICMA_LIBRARIES_DEP)
    list(APPEND LIBS ${HICMA_LIBRARIES_DEP})
else ()
    list(APPEND LIBS ${HICMA_LIBRARIES})
endif()

message(STATUS "HiCMA done")
