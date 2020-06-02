#include <pybind11/pybind11.h>
#include <rdapi/rdapi.h>

namespace py = pybind11;

extern void bindRDPython(py::module&);

PYBIND11_MODULE(rdpython, m)
{
  py::class_<RDDatabase>(m, "RDDatabase");
  py::class_<RDContext>(m, "RDContext");
  py::class_<RDLoader>(m, "RDLoader");
  py::class_<RDObject>(m, "RDObject");
  py::class_<RDBuffer>(m, "RDBuffer");
  py::class_<RDBufferView>(m, "RDBufferView");
  py::class_<RDBlockContainer>(m, "RDBlockContainer");
  py::class_<RDGraph>(m, "RDGraph");
  py::class_<RDFunctionBasicBlock>(m, "RDFunctionBasicBlock");
  py::class_<RDCursor>(m, "RDCursor");
  py::class_<RDDocument>(m, "RDDocument");
  py::class_<RDDisassembler>(m, "RDDisassembler");
  py::class_<RDRenderer>(m, "RDRenderer");
  py::class_<RDRendererItem>(m, "RDRendererItem");

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
