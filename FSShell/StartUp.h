#pragma once


// CStartUp dialog

class CStartUp : public CDialogEx
{
    DECLARE_DYNAMIC(CStartUp)

public:
    CStartUp(CWnd* pParent = NULL);   // standard constructor
    virtual ~CStartUp();

// Dialog Data
    enum { IDD = IDD_StartUp };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    int m_DiskSizeKB;
	CString m_FileName;
    afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButtonOpenfromfile();
};
