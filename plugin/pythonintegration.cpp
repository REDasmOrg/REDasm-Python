#include "pythonintegration.h"
#include <redasm/support/filesystem.h>
#include <redasm/context.h>

#define PYTHON_PLUGIN_FOLDER "python"
#define PYTHON_PLUGIN_EXT    ".py"

extern "C" PyObject* PyInit_REDasmPython();

PythonIntegration::PythonIntegration()
{
  r_ctx->log("Python Plugin Loaded");
  Py_Initialize();

  if(PyInit_REDasmPython()) this->findPlugins();
  else r_ctx->log("Failed to initialize the REDasmPython");
}

PythonIntegration::~PythonIntegration()
{
  Py_FinalizeEx();
  r_ctx->log("Python Plugin Unloaded");
}

void PythonIntegration::appendPath(const REDasm::String &pluginpath)
{
  PyObject* pathobj = PySys_GetObject("path");
  PyList_Append(pathobj, PyUnicode_FromString(pluginpath.c_str()));
}

void PythonIntegration::execPlugin(PyObject* pluginobj)
{
  PyObject* pluginep = PyObject_GetAttrString(pluginobj, "redasm_init_plugin");
  PyObject* result = PyObject_CallFunctionObjArgs(pluginep, nullptr);

  if(result) Py_DECREF(result);
  Py_DECREF(pluginep);
}

void PythonIntegration::loadPlugins(const REDasm::String& pluginpath)
{
  REDasm::list_adapter_ptr<REDasm::FS::Entry> entries(REDasm::FS::recurse(pluginpath));
  
  for(size_t i = 0; i < entries->size(); i++)
  {
    const REDasm::FS::Entry& entry = entries->at(i);
    if(entry.path.ext() != PYTHON_PLUGIN_EXT) continue;

    PyObject* pluginobj = PyImport_ImportModule(entry.path.stem().c_str());

    if(!pluginobj)
    {
      r_ctx->log("Cannot load " + entry.path.name().quoted());
      continue;
    }

    this->execPlugin(pluginobj);
    Py_DECREF(pluginobj);
  }
 }

void PythonIntegration::findPlugins()
{
  REDasm::list_adapter_ptr<REDasm::String> adapter(r_ctx->pluginPaths());

  for(size_t i = 0; i < adapter->size(); i++)
  {
    REDasm::String pluginpath = REDasm::FS::Path::join(adapter->at(i), PYTHON_PLUGIN_FOLDER);
    this->appendPath(pluginpath);
    this->loadPlugins(pluginpath);
  }
}