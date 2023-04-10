
# Copyright (c) 2017-2023 King Abdullah University of Science and Technology,
# Copyright (c) 2023 by Brightskies inc,
# All rights reserved.
# ExaGeoStat is a software package, provided by King Abdullah University of Science and Technology (KAUST).

# @file ImportNLOPT.cmake
# @version 1.0.0
# @author Sameh Abdulah
# @date 2023-03-26
message("")
message("---------------------------------------- NLOPT")
message(STATUS "Checking for NLOPT")
include(macros/BuildDependency)

if (NOT TARGET NLOPT_FOUND)
    include(FindPkgConfig)
    find_package(PkgConfig QUIET)
    find_package(NLOPT 2.4.2 QUIET)

    if (NLOPT_FOUND)
        message("   Found NLOPT: ${NLOPT_LIBRARIES}")
    else()
        message("   Can't find NLOPT, Installing it instead ..")
        set(FLAGS -DCMAKE_INSTALL_PREFIX=${PROJECT_SOURCE_DIR}/installdir/_deps/NLOPT/)
        set(ISCMAKE ON)
        set(ISGIT ON)
        set(NLOPT_DIR  ${PROJECT_SOURCE_DIR}/installdir/_deps/NLOPT/)
        BuildDependency(NLOPT "https://github.com/stevengj/nlopt" "v2.7.1" ${FLAGS} ${ISCMAKE} ${ISGIT})
        set(NLOPT_LIBRARY_DIRS= ${PROJECT_SOURCE_DIR}/installdir/_deps/NLOPT/lib:$NLOPT_LIBRARY_DIRS)
        set(NLOPT_INCLUDE_DIRS= ${PROJECT_SOURCE_DIR}/installdir/_deps/NLOPT/include:$NLOPT_INCLUDE_DIRS)

        find_package(NLOPT 2.4.2 REQUIRED)
    endif()
else()
    message("   NLOPT already included")
endif()
include_directories(${NLOPT_INCLUDE_DIRS})
link_directories(${NLOPT_LIBRARY_DIRS})
list(APPEND LIBS ${NLOPT_LIBRARIES})