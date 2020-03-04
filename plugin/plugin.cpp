#include <redasm/redasm.h>
#include "pythonintegration.h"

REDASM_PLUGIN("Dax", "Python Plugin Loader", "MIT", 1)
REDASM_LOAD { python.plugin = new PythonIntegration(); return true; }
REDASM_UNLOAD { python.plugin->release(); }
