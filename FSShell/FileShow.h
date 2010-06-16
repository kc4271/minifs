#pragma once
#include "filesystem.h"

// CFileShow dialog

class CFileShow : public CDialogEx
{
	DECLARE_DYNAMIC(CFileShow)

public:
	CFileShow(CWnd* pParent = NULL);   // standard constructor
	CFileShow(CString fname,unsigned int handle,miniFileSystem *fs = NULL,CWnd* pParent = NULL);
	virtual ~CFileShow();
	
// Dialog Data
	enum { IDD = IDD_DIALOG_FILEPROPERTY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

public:
	void ShowContent();
	miniFileSystem *m_fs;
	unsigned int m_hFile;
	CString m_filename;
	LONGLONG m_filesize;
	CString m_filecontent;
	fileOpenTable *fot;
	UINT m_seek;
	afx_msg void OnBnClickedButton1();
};
