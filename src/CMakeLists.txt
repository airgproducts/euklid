set(CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})


set(CPP_FILES
        mesh/mesh.cpp
        vector/transform.cpp
        vector/vector.cpp
        polyline/polyline.cpp
        polyline/polyline_2d.cpp
        polyline/interpolation.cpp
        spline/basis.cpp
        spline/spline.cpp
        mesh/mesh.cpp
        plane/plane.cpp
        )

set(HEADER_FILES
        mesh/mesh.hpp
        vector/transform.hpp
        vector/vector.hpp
        polyline/polyline.hpp
        polyline/polyline_2d.hpp
        polyline/interpolation.hpp
        spline/basis.hpp
        spline/spline.hpp
        plane/plane.hpp
        )


pybind11_add_module(euklid bindings/py/pybind.cpp ${CPP_FILES} ${HEADER_FILES})

if(OpenMP_CXX_FOUND)
    target_link_libraries(euklid PUBLIC OpenMP::OpenMP_CXX)
endif()

target_link_libraries(euklid PUBLIC Eigen3::Eigen)
