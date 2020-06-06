#include <pybind11/pybind11.h>
#include <rdapi/rdapi.h>
#include <rdapi_loader.h>
#include <rdapi_assembler.h>
#include <rdapi_command.h>
#include <string.h>

namespace py = pybind11;

extern void bindRDPython(py::module&);

PyRDLoaderPlugin rd_loader_plugin(const char* name)
{
  PyRDLoaderPlugin cmd;
  RD_PLUGIN_INIT(cmd, strdup(name)); // NOTE: Leaks memory
  return cmd;
}

PyRDAssemblerPlugin rd_assembler_plugin(const char* name)
{
  PyRDAssemblerPlugin cmd;
  RD_PLUGIN_INIT(cmd, strdup(name)); // NOTE: Leaks memory
  return cmd;
}

PyRDCommandPlugin rd_command_plugin(const char* name)
{
  PyRDCommandPlugin cmd;
  RD_PLUGIN_INIT(cmd, strdup(name)); // NOTE: Leaks memory
  return cmd;
}

PYBIND11_MODULE(rdpython, m)
{
  m.def("rd_loader_plugin", &rd_loader_plugin);
  m.def("rd_assembler_plugin", &rd_assembler_plugin);
  m.def("rd_command_plugin", &rd_command_plugin);

  bindRDPython(m);
}
