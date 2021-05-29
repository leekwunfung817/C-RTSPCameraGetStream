
// PythonSocket.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "PySockResource.h"		// main symbols


// CPythonSocketApp:
// See PythonSocket.cpp for the implementation of this class
//

class CPythonSocketApp : public CWinApp
{
public:
	CPythonSocketApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CPythonSocketApp theApp;