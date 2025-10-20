CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

PROJECT(euklid)

# OPTIONS
OPTION(BUILD_PYTHON_BINDINGS "create python bindings" ON)
OPTION(WALL "Treat all warnings as errors" ON)
OPTION(USE_SUBMODULE_LIBS "Use libs from submodules" ON)

set(CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

set (CMAKE_CXX_STANDARD 20)
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")

if(${USE_SUBMODULE_LIBS} AND EXISTS ${PROJECT_SOURCE_DIR}/lib/pybind11/CMakeLists.txt)
    add_subdirectory(${PROJECT_SOURCE_DIR}/lib/pybind11)
else()
    find_package(pybind11 REQUIRED)
endif()
set(PYBIND11_CPP_STANDARD -std=c++14)


if(${USE_SUBMODULE_LIBS} AND EXISTS ${PROJECT_SOURCE_DIR}/lib/eigen/CMakeLists.txt)
    add_subdirectory(${PROJECT_SOURCE_DIR}/lib/eigen)
else()
    find_package (Eigen3 3.4 REQUIRED NO_MODULE)
endif()

find_package(OpenMP)



if (WALL)
    message("Treating all warnings as errors")
    if (MSVC)
        # warning level 4 and all warnings as errors
        # add_compile_options(/W4 /WX)
    else()
        # lots of warnings and all warnings as errors
        # TODO: add -Wall ()
        add_compile_options(-Wall -Wextra -pedantic -Werror)
        if(UNIX AND NOT APPLE)
            add_compile_options(-Wno-deprecated-enum-enum-conversion)
        endif()
    endif()
endif()

ADD_SUBDIRECTORY("src" "euklid")
