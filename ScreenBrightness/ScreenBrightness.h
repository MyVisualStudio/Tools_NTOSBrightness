
// ScreenBrightness.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CScreenBrightnessApp: 
// �йش����ʵ�֣������ ScreenBrightness.cpp
//

class CScreenBrightnessApp : public CWinApp
{
public:
	CScreenBrightnessApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CScreenBrightnessApp theApp;