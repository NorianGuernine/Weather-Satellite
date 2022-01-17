#include <stdio.h>
#include <Python.h>

void main(void) {
	Py_Initialize();
	PyObject *pName,*pName2, *pModule, *pFunc, *pArgs, *pValue;
    	/* add . to the path */
	PyObject* sys = PyImport_ImportModule("sys");
	PyObject* path = PyObject_GetAttrString(sys, "path");
	PyList_Insert(path, 0, PyUnicode_FromString("."));
	//PySys_SetPath(L".");
	pName = PyUnicode_FromString((char*)"main");  	
	pModule = PyImport_Import(pName);	
	pFunc = PyObject_GetAttrString(pModule, (char*)"main");      
	pArgs = Py_BuildValue("(s)",(char *)"137912500"); 
	pValue = PyObject_CallObject(pFunc, pArgs);                  
	Py_Finalize(); 
	return;
}
