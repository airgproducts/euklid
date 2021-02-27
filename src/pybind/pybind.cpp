#include <pybind11/pybind11.h>
#include <vector>
#include <pybind11/stl.h>

#include "version.hpp"
#include "pybind/mesh.cpp"
#include "pybind/vector.cpp"


namespace py = pybind11;
using namespace py::literals;


PYBIND11_MODULE(euklid, m) {
    m.doc() = "euklid vector operations";

    euklid::vector::REGISTER(m);
    euklid::mesh::REGISTER(m);


    m.attr("__version__") = py::str(euklid::version);
}