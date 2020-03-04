#pragma once

#include <redasm/plugins/plugin.h>

class PythonIntegration: public REDasm::Plugin
{
  public:
    PythonIntegration();
    ~PythonIntegration();

  private:
    void findPlugins();
};