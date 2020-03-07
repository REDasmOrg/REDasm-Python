#pragma once

#include <redasm/plugins/plugin.h>
#include <Python.h>

class PythonIntegration: public REDasm::Plugin
{
  public:
    PythonIntegration();
    ~PythonIntegration();

  private:
    void execPlugin(PyObject* pluginobj);
    void loadPlugins(const REDasm::String& pluginpath);
    void appendPath(const REDasm::String& pluginpath);
    void findPlugins();
};