
// FSShellDlg.h : ͷ�ļ�
//

#pragma once
#include "filesystem.h"
#include "afxwin.h"

class CFSShellDlg;

void ShowInfo(const char *s,CFSShellDlg *dlg);

// CFSShellDlg �Ի���
class CFSShellDlg : public CDialogEx
{
// ����
public:
    CFSShellDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
    enum { IDD = IDD_FSSHELL_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
    HICON m_hIcon;


    // ���ɵ���Ϣӳ�亯��
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
