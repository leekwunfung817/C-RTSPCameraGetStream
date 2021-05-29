
// CPosRtspDemo.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "RTSPResource.h"		// main symbols


// CCPosRtspDemoApp:
// See CPosRtspDemo.cpp for the implementation of this class
//

class CCPosRtspDemoApp : public CWinApp
{
public:
	CCPosRtspDemoApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CCPosRtspDemoApp theApp;