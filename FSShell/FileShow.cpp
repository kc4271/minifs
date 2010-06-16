// FileShow.cpp : implementation file
//

#include "stdafx.h"
#include "FSShell.h"
#include "FileShow.h"
#include "afxdialogex.h"

static string Wchar2Schar(const CString &s)
{
    wchar_t *buf_w = new wchar_t[s.GetLength() + 1];
    wcscpy_s(buf_w,s.GetLength() + 1,(LPCTSTR)s);
    char *buf = new char[(s.GetLength() + 1) * 2];
    int conv = WideCharToMultiByte(CP_ACP,0,buf_w,wcslen(buf_w),buf,(s.GetLength() + 1) * 2,NULL,NULL);
    buf[conv] = '\0';
    string t(buf);
    delete []buf;
    delete []buf_w;
    return t;
}

static CString HexConv(unsigned char c)
{
	int high = (c>>4);
	int low = c & 0xF;
	string t;
	if(high < 10)
		t += '0' + high;
	else
		t += 'A' + high - 10;
	if(low < 10)
		t += '0' + low;
	else
		t += 'A' + low - 10;
	CString str(t.c_str());
	return str;
}

static char CharProcess(char c)
{
	if(c <= 0)
		c = '.';
	else
	{
		if(!isgraph(c))
			c = '.';
		if(isspace(c) && c != ' ')
			c = '.';
	}
	return c;
}

// CFileShow dialog

IMPLEMENT_DYNAMIC(CFileShow, CDialogEx)

CFileShow::CFileShow(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFileShow::IDD, pParent)
	, m_filename(_T(""))
	, m_filesize(0)
	, m_filecontent(_T(""))
	, m_seek(0)
{

}

CFileShow::CFileShow(CString fname,unsigned int handle,miniFileSystem *fs,CWnd* pParent)
	: CDialogEx(CFileShow::IDD, pParent)
	, m_filename(fname)
	, m_filesize(0)
	, m_fs(fs)
	, m_hFile(handle)
{
	m_seek = 0;
	Disk &disk = m_fs->dskmounted[m_fs->curr];
	m_filesize = disk.get_file_size(m_hFile);
	fot = &disk.fopt.find(m_hFile)->second;
	
	ShowContent();
}

CFileShow::~CFileShow()
{

}

BOOL CFileShow::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	this->SetWindowText(m_filename);
	
	return TRUE;
}


void CFileShow::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT2, m_filesize);
	DDX_Text(pDX, IDC_EDIT_FILECONTENT, m_filecontent);
	DDX_Text(pDX, IDC_EDIT1, m_seek);
}

void CFileShow::ShowContent()
{
	m_filecontent = "";
	Disk &disk = m_fs->dskmounted[m_fs->curr];
	disk.lseek_file(m_hFile,m_seek);
	char *ptr = fot->ptrinbuf;
	UINT i = ptr - fot->buf;
	UINT offset = m_seek;
	while(ptr != fot->buf + BLOCKSIZE_KB * KBSIZE && offset != m_filesize)
	{
		CString str;
		if(fot->buf + BLOCKSIZE_KB * KBSIZE - ptr >= 8 && m_filesize - offset >= 8)
		{
			str.Format(TEXT("%012d:  %3s %3s %3s %3s %3s %3s %3s %3s ; %c%c%c%c%c%c%c%c \r\n"),
				offset,
				HexConv(fot->buf[i]),HexConv(fot->buf[i+1]),HexConv(fot->buf[i+2]),
				HexConv(fot->buf[i+3]),HexConv(fot->buf[i+4]),HexConv(fot->buf[i+5]),
				HexConv(fot->buf[i+6]),HexConv(fot->buf[i+7]),
				CharProcess(fot->buf[i]),CharProcess(fot->buf[i+1]),CharProcess(fot->buf[i+2]),
				CharProcess(fot->buf[i+3]),CharProcess(fot->buf[i+4]),CharProcess(fot->buf[i+5]),
				CharProcess(fot->buf[i+6]),CharProcess(fot->buf[i+7]));
			i += 8;
			ptr += 8;
			offset += 8;
			m_filecontent += str;
		}
		else
		{
			UINT N = min(UINT((fot->buf + BLOCKSIZE_KB * KBSIZE) - ptr),UINT(m_filesize - offset));
			N = 8 - N;
			str.Format(TEXT("%012d:  "),offset);
			CString head,tail;
			while(ptr != fot->buf + BLOCKSIZE_KB * KBSIZE && offset != m_filesize)
			{
				CString t;
				t.Format(TEXT("%3s "),HexConv(fot->buf[i]));
				head += t;
				t.Format(TEXT("%c"),CharProcess(fot->buf[i]));
				tail += t;
				i++;
				offset++;
				ptr++;
			}
			while(N--)
			{
				head += TEXT("    ");
				tail += TEXT(" ");
			}
			m_filecontent += str + head + TEXT("; ") + tail + TEXT(" \r\n");
		}
	}
	if(offset != m_filesize)
	{
		m_filecontent += TEXT("                只显示当前Seek到块尾间的数据。");
	}
	else
	{
		m_filecontent += TEXT("                到达文件尾。");
	}
}


BEGIN_MESSAGE_MAP(CFileShow, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CFileShow::OnBnClickedButton1)
END_MESSAGE_MAP()


// CFileShow message handlers


void CFileShow::OnBnClickedButton1()
{
	UINT orig = m_seek;
	UpdateData();
	if(m_seek >= m_filesize)
	{
		::AfxMessageBox(TEXT("输入值过大，请重新输入！"));
		m_seek = orig;
	}
	else if(orig == m_seek)
	{
		return;
	}
	else
	{
		ShowContent();
	}
	UpdateData(FALSE);
}
