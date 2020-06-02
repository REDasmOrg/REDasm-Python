#include <rdapi/rdapi.h>
#include "pythonintegration.h"

void redasm_entry()
{
  static PythonIntegration rdpy;

  RD_PLUGIN_CREATE(RDCommandPlugin, rdpython, "REDasm Python Integration");
  RDCommand_Register(&rdpython);
}

