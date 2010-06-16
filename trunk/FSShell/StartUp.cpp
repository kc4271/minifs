// StartUp.cpp : implementation file
//

#include "stdafx.h"
#include "FSShell.h"
#include "StartUp.h"
#include "afxdialogex.h"


// CStartUp dialog

IMPLEMENT_DYNAMIC(CStartUp, CDialogEx)

CStartUp::CStartUp(CWnd* pParent /*=NULL*/)
        : CDialogEx(CStartUp::IDD, pParent)
        , m_DiskSizeKB(32768)
{

}

CStartUp::~CStartUp()
{
}

void CStartUp::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT1, m_DiskSizeKB);
}


BEGIN_MESSAGE_MAP(CStartUp, CDialogEx)
    ON_BN_CLICKED(IDOK, &CStartUp::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CStartUp::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON_OPENFROMFILE, &CStartUp::OnBnClickedButtonOpenfromfile)
END_MESSAGE_MAP()


// CStartUp message handlers


void CStartUp::OnBnClickedOk()
{
    // TODO: Add your control notification handler code here
    UpdateData();
    if (this->m_DiskSizeKB <= 0)
    {
        ::AfxMessageBox(TEXT("磁盘大小必须为正数"));
        return;
    }
    CDialogEx::OnOK();
}


void CStartUp::OnBnClickedButton1()
{
	
}


void CStartUp::OnBnClickedButtonOpenfromfile()
{
	CFileDialog dlg(TRUE, _T("dsk" ), _T("*.dsk"), OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		_T("虚拟磁盘|*.dsk|所有文件|*||"));
	if (dlg.DoModal() == IDOK)
	{
		this->m_FileName = dlg.GetPathName();
		CDialogEx::OnOK();
	}
}
