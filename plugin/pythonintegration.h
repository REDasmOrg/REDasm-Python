#pragma once

#include <rdapi/rdapi.h>
#include <pybind11/pybind11.h>

class PythonIntegration
{
  public:
    PythonIntegration();
    ~PythonIntegration();

  private:
    void addSearchPaths();
    void loadPlugins(const std::string& pluginpath);
    void execPlugin(pybind11::object& pluginobj);
    void findPlugins();

  private:
    pybind11::object m_rdpython; 
};
