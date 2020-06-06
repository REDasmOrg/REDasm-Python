#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <rdapi/rdapi.h>
#include <rdapi_loader.h>
#include <rdapi_assembler.h>
#include <rdapi_command.h>
#include <iostream>
#include <string.h>

namespace py = pybind11;

extern void bindRDPython(py::module&);

static py::object userdata_to_obj(void* userdata)
{
  PyObject* rawobj = reinterpret_cast<PyObject*>(userdata);
  return py::reinterpret_borrow<py::object>(rawobj);
}

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

  m.def("RDRenderer_GetItems", [](const RDRenderer* r, size_t index, size_t count, const py::object& obj) {
      RDRenderer_GetItems(r, index, count, [](const RDRendererItem* ritem, size_t index, void* userdata) {
          py::object obj = userdata_to_obj(userdata);
          obj(ritem, index);
      }, obj.ptr());
  });

  m.def("RD_GetLoaders", [](const RDLoaderRequest* loadrequest, const py::object& obj) {
      RD_GetLoaders(loadrequest, [](RDLoaderPlugin* plugin, void* userdata) {
          py::object obj = userdata_to_obj(userdata);
          obj(static_cast<PyRDLoaderPlugin*>(plugin));
      }, obj.ptr());
  });

  m.def("RD_GetAssemblers", [](const py::object& obj) {
      RD_GetAssemblers([](RDAssemblerPlugin* plugin, void* userdata) {
          py::object obj = userdata_to_obj(userdata);
          obj(static_cast<PyRDAssemblerPlugin*>(plugin));
      }, obj.ptr());
  });
  
  m.def("RDEvent_Subscribe", [](const py::object& obj) {
      return RDEvent_Subscribe(obj.ptr(), [](const RDEventArgs* e) {
          py::object obj = userdata_to_obj(e->owner);
          obj(e);
          }, nullptr);
  });

  m.def("RDEvent_Unsubscribe", [](const py::object& obj) {
      RDEvent_Unsubscribe(obj.ptr());
  });

  bindRDPython(m);
}
