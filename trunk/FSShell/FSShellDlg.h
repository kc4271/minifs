
// FSShellDlg.h : 头文件
//

#pragma once
#include "filesystem.h"
#include "afxwin.h"

class CFSShellDlg;

void ShowInfo(const char *s,CFSShellDlg *dlg);

// CFSShellDlg 对话框
class CFSShellDlg : public CDialogEx
{
// 构造
public:
    CFSShellDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
    enum { IDD = IDD_FSSHELL_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
    HICON m_hIcon;


    // 生成的消息映射函数
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()
public:
    void DrawBitMap(vector<COLOR> v);
    friend void ShowInfo(const char *s,CFSShellDlg *dlg);
    CString m_sErrorString;

    miniFileSystem m_fs;

    CListBox m_DirList;
    void OnClose();
    afx_msg void OnBnClickedOk();
    afx_msg void OnLbnSelchangeListDir();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedButton1();
    afx_msg void OnDropFiles(HDROP hDropInfo);
	int m_DiskSizeKB;
	afx_msg void OnLbnDblclkListDir();
};
