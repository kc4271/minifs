
// FSShell.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CFSShellApp:
// �йش����ʵ�֣������ FSShell.cpp
//

class CFSShellApp : public CWinApp
{
public:
    CFSShellApp();

// ��д
public:
    virtual BOOL InitInstance();

// ʵ��

    DECLARE_MESSAGE_MAP()
};

extern CFSShellApp theApp;