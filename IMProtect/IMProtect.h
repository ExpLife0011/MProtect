
// IMProtect.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CIMProtectApp: 
// �йش����ʵ�֣������ IMProtect.cpp
//

class CIMProtectApp : public CWinApp
{
public:
	CIMProtectApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CIMProtectApp theApp;