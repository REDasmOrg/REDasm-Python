#include <pybind11/pybind11.h>
#include <rdapi/rdapi.h>

namespace py = pybind11;

extern void bindRDPython(py::module&);

PYBIND11_MODULE(rdpython, m)
{
  bindRDPython(m);
}
