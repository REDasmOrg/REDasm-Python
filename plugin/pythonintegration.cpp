#include "pythonintegration.h"
#include <pybind11/embed.h>
#include <filesystem>

#define PYTHON_PLUGIN_FOLDER "python"
#define PYTHON_PLUGIN_EXT    ".py"

namespace fs = std::filesystem;
namespace py = pybind11;

extern "C" PyObject* PyInit_rdpython();

PythonIntegration::PythonIntegration()
{
 py::initialize_interpreter();
 m_rdpython = py::reinterpret_borrow<py::object>(PyInit_rdpython());

 if(m_rdpython)
 {
  this->addSearchPaths();
  this->findPlugins();
  return;
 }

 rd_log("Failed to initialize the REDasmPython");
}

PythonIntegration::~PythonIntegration() { py::finalize_interpreter(); }

void PythonIntegration::addSearchPaths()
{
 RD_GetPluginPaths([](const char* pluginpath, void*) {
   py::object pathobj = py::module::import("sys").attr("path");
   pathobj.attr("append")((fs::path(pluginpath) / PYTHON_PLUGIN_FOLDER).string());
   }, nullptr);
}

void PythonIntegration::execPlugin(py::object& pluginobj)
{
  py::object pluginep = pluginobj.attr("redasm_entry");
  if(pluginep) pluginep();
}

void PythonIntegration::loadPlugins(const std::string& pluginpath)
{
 if(!fs::is_directory(pluginpath)) return;

 for(const auto& item : fs::recursive_directory_iterator(pluginpath))
 {
  if(!fs::is_regular_file(item) && !fs::is_symlink(item)) continue;
  if(item.path().extension() != PYTHON_PLUGIN_EXT) continue;

  try {
   py::module pluginobj = py::module::import(item.path().stem().c_str());
   pluginobj.add_object("rdpython", m_rdpython);

   if(!pluginobj)
   {
    rd_log("Cannot load " + item.path().string());
    continue;
   }

   this->execPlugin(pluginobj);
  } catch(py::error_already_set& e) {
   rd_log(e.what());
  }
 }
}

void PythonIntegration::findPlugins()
{
 RD_GetPluginPaths([](const char* pluginpath, void* userdata) {
   auto* thethis = reinterpret_cast<PythonIntegration*>(userdata);
   thethis->loadPlugins(fs::path(pluginpath) / PYTHON_PLUGIN_FOLDER);
 }, this);
}
