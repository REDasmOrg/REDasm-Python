#include <rdapi/rdapi.h>
#include "pythonintegration.h"

static PythonIntegration rdpython;

void redasm_entry()
{
  RD_PLUGIN_CREATE(RDCommandPlugin, rdpython, "REDasm Python Integration");
  RDCommand_Register(&rdpython);
}

