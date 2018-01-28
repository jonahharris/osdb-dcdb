/* Interface File: conttcl.i */

%module conttcl
%{

#define SWIGCODE
#include <container.h>

#define SWIG_RcFileName "~/.wishrc"
%}

%include tclsh.i

/*
%typemap(out) char * {
  if (result == NULL) {
    Tcl_SetStringObj ($result, container_error, -1);
    SWIG_exception_fail(-1, container_error);
    return TCL_ERROR;
  }
  else {
    Tcl_SetStringObj ($result, result, -1);
    return TCL_OK;
  }
}


%typemap(out) int {
  if (result == _ERROR_)  {
    Tcl_SetStringObj ($result, container_error, -1);
    SWIG_exception_fail(-1, container_error);
    return TCL_ERROR;
  }
  else    {
    Tcl_SetObjResult (interp, Tcl_NewIntObj((long) result));
    return TCL_OK;
  }
}
*/

%inline %{
char *containerInit (char *fname);
int containerDelete (char *handle);
void handleDelete (contHandle *hdl);
int containerClearError (void);
int containerSetInt (char *handle, char *field, int value);
int containerSetLong (char *handle, char *field, long value);
int containerSetFloat (char *handle, char *field, float value);
int containerSetDouble (char *handle, char *field, double value);
int containerSetString (char *handle, char *field, char *value);
char *containerGetField (char *handle, char *field);
int containerAddRecord (char *handle);
int containerSearch (char *handle, char *value);
int containerQuery (char *handle, char *value);
int containerDeleteRecord (char *handle, char *value);
int containerFirst (char *handle);
int containerLast (char *handle);
int containerPrev (char *handle);
int containerNext (char *handle);
int containerBOF (char *handle);
int containerEOF (char *handle);
int containerNumRecords (char *handle);
int containerRestructureIndex (char *handle);
%}
