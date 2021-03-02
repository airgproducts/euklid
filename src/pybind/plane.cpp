#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>

#include "plane/plane.hpp"

namespace py = pybind11;
using namespace py::literals;


namespace euklid::plane {

    void REGISTER(pybind11::module module) {
        pybind11::module m = module.def_submodule("plane");


        py::class_<Plane>(m, "Plane")
            .def(py::init<Transformation>())
            .def(py::init<Vector3D, Vector3D, Vector3D>())
            .def("project", py::overload_cast<const Vector3D&>(&Plane::project, py::const_))
            .def("project", py::overload_cast<const PolyLine3D&>(&Plane::project, py::const_));

    }
}