#pragma once

#include <rdapi/rdapi.h>
#include <Python.h>

class PythonIntegration
{
  public:
    PythonIntegration();
    ~PythonIntegration();

  private:
    void execPlugin(PyObject* pluginobj);
    void loadPlugins();
    void appendPath();
    void findPlugins();
};
