#include <pybind11/pybind11.h>
#include <rdapi/rdapi.h>

namespace py = pybind11;

extern void bindRDPython(py::module&);

PYBIND11_MODULE(rdpython, m)
{
  py::class_<RDLocation>(m, "RDLocation")
    .def(py::init<>())
    .def_readwrite("value", &RDLocation::value)
    .def_readwrite("offset", &RDLocation::offset)
    .def_readwrite("address", &RDLocation::address)
    .def_readwrite("valid", &RDLocation::valid);

  py::class_<RDSegment>(m, "RDSegment")
    .def(py::init<>())
    .def_readonly("name", &RDSegment::name)
    .def_readwrite("offset", &RDSegment::offset)
    .def_readwrite("endoffset", &RDSegment::endoffset)
    .def_readwrite("address", &RDSegment::address)
    .def_readwrite("endaddress", &RDSegment::endaddress)
    .def_readwrite("flags", &RDSegment::flags)
    .def_readwrite("coveragebytes", &RDSegment::coveragebytes)
    .def_readwrite("itemscount", &RDSegment::itemscount);

  bindRDPython(m);
}
