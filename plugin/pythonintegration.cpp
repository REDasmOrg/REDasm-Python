#include "pythonintegration.h"
#include <pybind11/pybind11.h>

#define PYTHON_PLUGIN_FOLDER "python"
#define PYTHON_PLUGIN_EXT    ".py"

namespace py = pybind11;

extern "C" PyObject* PyInit_rdpython();

PythonIntegration::PythonIntegration()
{
 rd_log("Python Plugin Loaded");
 Py_Initialize();

 if(PyInit_rdpython()) this->findPlugins();
 else rd_log("Failed to initialize the REDasmPython");
}

PythonIntegration::~PythonIntegration()
{
 Py_FinalizeEx();
 rd_log("Python Plugin Unloaded");
}

void PythonIntegration::appendPath()
{
 RD_GetPluginPaths([](const char* pluginpath, void*) {
   PyObject* pathobj = PySys_GetObject("path");
   rd_log(pluginpath);
   PyList_Append(pathobj, PyUnicode_FromString(pluginpath));
   }, nullptr);
}

void PythonIntegration::execPlugin(PyObject* pluginobj)
{
 PyObject* pluginep = PyObject_GetAttrString(pluginobj, "redasm_entry");
 PyObject* result = PyObject_CallFunctionObjArgs(pluginep, nullptr);

 if(result) Py_DECREF(result);
 Py_DECREF(pluginep);
}

void PythonIntegration::loadPlugins()
{
 //REDasm::list_adapter_ptr<REDasm::FS::Entry> entries(REDasm::FS::recurse(pluginpath));

 //for(size_t i = 0; i < entries->size(); i++)
 //{
 //const REDasm::FS::Entry& entry = entries->at(i);
 //if(entry.path.ext() != PYTHON_PLUGIN_EXT) continue;

 //PyObject* pluginobj = PyImport_ImportModule(entry.path.stem().c_str());

 //if(!pluginobj)
 //{
 //r_ctx->log("Cannot load " + entry.path.name().quoted());
 //continue;
 //}

 //this->execPlugin(pluginobj);
 //Py_DECREF(pluginobj);
 //}
}

void PythonIntegration::findPlugins()
{
 //REDasm::list_adapter_ptr<REDasm::String> adapter(r_ctx->pluginPaths());

 //for(size_t i = 0; i < adapter->size(); i++)
 //{
 //REDasm::String pluginpath = REDasm::FS::Path::join(adapter->at(i), PYTHON_PLUGIN_FOLDER);
 //this->appendPath(pluginpath);
 //this->loadPlugins(pluginpath);
 //}
}
