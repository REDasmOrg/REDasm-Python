#include "pythonintegration.h"
#include <redasm/context.h>
#include <Python.h>

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

void PythonIntegration::findPlugins()
{

}