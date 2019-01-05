/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "py_zipfile.h"
#include "resmgr/resmgr.h"

namespace KBEngine{ 
namespace script{

bool PyZipFile::isInit = false;

//-------------------------------------------------------------------------------------
bool PyZipFile::initialize(void)
{
	if(isInit)
		return true;

	return isInit;
}

//-------------------------------------------------------------------------------------
void PyZipFile::finalise(void)
{
}

//-------------------------------------------------------------------------------------
bool PyZipFile::compressDirectory(const std::string& sourceDir, const std::string& outfile)
{
	std::string outpath = "", zipfile = outfile;

	std::vector<std::string> tmpvec;
	KBEngine::strutil::kbe_splits(zipfile, ".", tmpvec);
	outpath = tmpvec[0];
	zipfile = tmpvec[1];

	if (zipfile.size() > 0)
	{
		PyObject* pyModule = PyImport_ImportModule("zipfile");
		if (pyModule)
		{
			PyObject* zipModule  = PyObject_GetAttrString(pyModule, "ZipFile");
			PyObject* zipTypeOBJ = PyObject_GetAttrString(pyModule, "ZIP_DEFLATED");
			Py_DECREF(pyModule);

			if (!zipModule || !zipTypeOBJ)
			{
				Py_XDECREF(zipTypeOBJ);
				Py_XDECREF(zipModule);
				goto _PYZIPMODULE_ERROR;
			}

			int ziptype = PyLong_AsLong(zipTypeOBJ);
			Py_XDECREF(zipTypeOBJ);

			PyObject* pyargs = PyTuple_New(3);
			PyTuple_SET_ITEM(pyargs, 0, PyUnicode_FromString((outpath + "." + zipfile).c_str()));
			PyTuple_SET_ITEM(pyargs, 1, PyUnicode_FromString("w"));
			PyTuple_SET_ITEM(pyargs, 2, PyLong_FromLong(ziptype));
			
			PyObject* zipObject = PyObject_CallObject(zipModule, pyargs);
			Py_DECREF(pyargs);
			Py_XDECREF(zipModule);

			if (zipObject == NULL)
			{
				SCRIPT_ERROR_CHECK();
				goto _PYZIPMODULE_ERROR;
			}

			wchar_t* wpath = strutil::char2wchar(sourceDir.c_str());
			std::vector<std::wstring> results;
			Resmgr::getSingleton().listPathRes(wpath, L"*", results);

			std::vector<std::wstring>::iterator iter = results.begin();
			for (; iter != results.end(); ++iter)
			{
				std::wstring wstrpath = (*iter);
				strutil::kbe_replace(wstrpath, wpath, L"");
				PyObject* pyResult = PyObject_CallMethod(zipObject, const_cast<char*>("write"),
					const_cast<char*>("u#u#"), (*iter).c_str(), (*iter).size(), wstrpath.c_str(), wstrpath.size());

				if (pyResult != NULL)
					Py_DECREF(pyResult);
				else
					SCRIPT_ERROR_CHECK();
			}

			PyObject* pyResult = PyObject_CallMethod(zipObject, const_cast<char*>("close"),
				const_cast<char*>(""));

			if (pyResult != NULL)
				Py_DECREF(pyResult);
			else
				SCRIPT_ERROR_CHECK();

			free(wpath);
			Py_DECREF(zipObject);
			return true;
		}
		else
		{
		_PYZIPMODULE_ERROR:
			SCRIPT_ERROR_CHECK();
		}
	}

	return false;
}


//-------------------------------------------------------------------------------------

}
}