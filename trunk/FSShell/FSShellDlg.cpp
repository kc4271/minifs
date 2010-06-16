
// FSShellDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "FSShell.h"
#include "FSShellDlg.h"
#include "afxdialogex.h"
#include "StartUp.h"
#include "FileShow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


static int nRow = 30;
static int nCol = 40;

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

CString GetDirPath()
{
	CString m_FileSrc;         //将选择的文件夹路径保存在此变量中
    TCHAR Buffer[MAX_PATH];
    
	BROWSEINFO bi;
    ZeroMemory(&bi, sizeof(BROWSEINFO));
    bi.hwndOwner = NULL;
    bi.ulFlags = BIF_RETURNONLYFSDIRS;     //要求返回文件系统的目录
    bi.pszDisplayName = Buffer;            //此参数如为NULL则不能显示对话框
    bi.lpszTitle = _T("请选择文件夹");
    bi.lpfn = NULL;
    bi.iImage=IDR_MAINFRAME;
    LPITEMIDLIST pIDList = SHBrowseForFolder(&bi);//调用显示选择对话框
    
	if (pIDList)
    {
        SHGetPathFromIDList(pIDList, Buffer);
        //取得文件夹路径到Buffer里
		m_FileSrc = Buffer;//将文件夹路径保存在一个CString对象里
	}
	LPMALLOC lpMalloc;
	if (FAILED(SHGetMalloc(&lpMalloc))) 
		return CString("");

    //释放内存
	lpMalloc->Free(pIDList);
	lpMalloc->Release();
	return m_FileSrc;
}

CString GetFileName(const wchar_t *pPath_w)
{
    wchar_t *fName_w = new wchar_t[wcslen(pPath_w) + 1];
    wchar_t fExt_w[20];
    _wsplitpath_s(pPath_w,NULL,0,NULL,0,fName_w,wcslen(pPath_w) + 1,fExt_w,20);
    wcscat_s(fName_w,wcslen(pPath_w) + 1,fExt_w);
    return CString(fName_w);
}

void ShowInfo(const char *s,CFSShellDlg *dlg)
{
    CString t(s);
    t += "\r\n";
    dlg->m_sErrorString = t +dlg->m_sErrorString;
    dlg->UpdateData(FALSE);
}

void Recurse(LPCTSTR pstr,CStringArray &sPathList)
{
    CFileFind finder;
    // build a string with wild cards
    CString strWildcard(pstr);
    strWildcard += _T("\\*.*");

    // start working for files
    BOOL bWorking = finder.FindFile(strWildcard);
    while (bWorking)
    {
        bWorking = finder.FindNextFile();
        if (finder.IsDots())
            continue;
        if (finder.IsDirectory())
        {
            CString str = finder.GetFilePath();
            TRACE(_T("%s\n"), (LPCTSTR)str);
            Recurse(str,sPathList);
        }
        else
        {
            sPathList.Add(finder.GetFilePath());
        }
    }
    finder.Close();
}

// CFSShellDlg 对话框

CFSShellDlg::CFSShellDlg(CWnd* pParent /*=NULL*/)
        : CDialogEx(CFSShellDlg::IDD, pParent)
        , m_sErrorString(_T(""))
		, m_DiskSizeKB(0)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFSShellDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ERROROUTPUT, m_sErrorString);
	DDX_Control(pDX, IDC_LIST_DIR, m_DirList);

	DDX_Text(pDX, IDC_EDIT2, m_DiskSizeKB);
}

BEGIN_MESSAGE_MAP(CFSShellDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON1, &CFSShellDlg::OnBnClickedButton1)
    ON_WM_DROPFILES()
    ON_BN_CLICKED(IDOK, &CFSShellDlg::OnBnClickedOk)
    ON_LBN_SELCHANGE(IDC_LIST_DIR, &CFSShellDlg::OnLbnSelchangeListDir)
    ON_BN_CLICKED(IDCANCEL, &CFSShellDlg::OnBnClickedCancel)
    ON_WM_CLOSE()
//	ON_LBN_DBLCLK(IDC_LIST_DIR, &CFSShellDlg::OnLbnDblclkListDir)
ON_LBN_DBLCLK(IDC_LIST_DIR, &CFSShellDlg::OnLbnDblclkListDir)
END_MESSAGE_MAP()


// CFSShellDlg 消息处理程序

BOOL CFSShellDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
    //  执行此操作
    SetIcon(m_hIcon, TRUE);			// 设置大图标
    SetIcon(m_hIcon, FALSE);		// 设置小图标

    // TODO: 在此添加额外的初始化代码
    CStartUp su;
    INT_PTR start = su.DoModal();
    if (start == IDOK)
    {
		if(su.m_FileName != "")
		{
			m_fs.dskmounted.push_back(Disk());
			m_fs.curr = 0;
			m_fs.dskmounted[0].init_from_file(Wchar2Schar(su.m_FileName).c_str());
			vector<string> Dir = m_fs.dskmounted[0].directory();
			for(unsigned int i = 0;i < Dir.size();i++)
			{
				CString str(Dir[i].c_str());
				m_DirList.AddString(str);
			}
			this->m_DiskSizeKB = (int)m_fs.dskmounted[0].disksize_KB;
		}
		else
		{
			if (!m_fs.create_disk("unname",(unsigned int)su.m_DiskSizeKB))
			{
				::AfxMessageBox(TEXT("Cann't create Disk"));
				exit(0);
			}
			else
			{
				this->m_DiskSizeKB = su.m_DiskSizeKB;
			}
		}
	}
	else
    {
        exit(0);
    }
	
	UpdateData(FALSE);
    m_fs.set_error_func(&ShowInfo,this);
    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。


void CFSShellDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // 用于绘制的设备上下文

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // 使图标在工作区矩形中居中
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // 绘制图标
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        this->DrawBitMap(m_fs.get_bit_map());
        CDialogEx::OnPaint();
    }
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CFSShellDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CFSShellDlg::OnBnClickedButton1()
{
    int nCount = m_DirList.GetSelCount();
	if(nCount > 0)
	{
		CString sDir = GetDirPath();
		if(sDir == "")
			return;
		sDir += '\\';
		int *CurSel = new int[nCount];
		m_DirList.GetSelItems(nCount,CurSel);
		for(int i = 0;i < nCount;i++)
		{
			CString name;
			m_DirList.GetText(CurSel[i],name);
			m_fs.save_file(Wchar2Schar(sDir).c_str(),Wchar2Schar(name).c_str());
		}
		delete []CurSel;
		ShowInfo("文件保存完成!",this);
	}
	else
	{
		::AfxMessageBox(TEXT("必须选择至少一个文件!"));
	}
}

void CFSShellDlg::DrawBitMap(vector<COLOR> v)
{
    CDC *cdc = GetDC();
    int w = 5;
    int h = 7;
    RECT rect;
    GetDlgItem(IDC_STATIC_GROUP)->GetWindowRect(&rect);
    ScreenToClient(&rect);
    CPoint point;
    int startY = rect.top + 15;
    int startX = rect.left + 10;

    unsigned int SUM = nRow * nCol;
    unsigned int index = 0;
    unsigned int rate = get_upbound(v.size(),SUM) / SUM;
    unsigned int pos = 0;
    while (true)
    {
        COLOR c = EMPTY;
        if (index == v.size())
            break;
        for (unsigned int z = 0; index < v.size() && z < rate; z++,index++)
        {
            if (v[index] > c)
                c = v[index];
        }

        int y = pos / nCol;
        int x = pos % nCol;
        point.y = startY + 15 * y;
        point.x = startX + 7 * x;
        CRect rc(point.x,point.y,point.x + w,point.y + h);
        DWORD color;
        switch (c)
        {
        case EMPTY:
            pos++;
            continue;
            break;
        case GERY:
            color = RGB(128,128,128);
            break;
        case RED:
            color = RGB(255,0,0);
            break;
        case GREEN:
            color = RGB(0,255,0);
            break;
        case YELLOW:
            color = RGB(255,255,0);
            break;
        }
        CBrush brush(color);
        cdc->FillRect(&rc,&brush);
        pos++;
    }

    while (pos < SUM)
    {
        int y = pos / nCol;
        int x = pos % nCol;
        point.y = startY + 15 * y;
        point.x = startX + 7 * x;
        CRect rc(point.x,point.y,point.x + w,point.y + h);
        CBrush brush(RGB(128,128,128));
        cdc->FillRect(&rc,&brush);
        pos++;
    }
    ReleaseDC(cdc);
}

afx_msg void CFSShellDlg::OnDropFiles(HDROP hDropInfo)
{
    int nFileCount = 0;
    CString strCount;
    CFileFind fileChecker;
    //取得被拖动文件的数目
    nFileCount=::DragQueryFile(hDropInfo,0xFFFFFFFF,NULL,512);

    for (int i = 0; i < nFileCount; i++)
    {
        //取得第i个拖动文件名所占字符数
        int NameSize = DragQueryFile(hDropInfo,i,NULL,0);
        HANDLE hHeap = GetProcessHeap();
        //根据字节数分配缓冲区
        LPWSTR pPath_w = (LPWSTR)HeapAlloc(hHeap,HEAP_ZERO_MEMORY,(NameSize + 1) * 2);
        if (pPath_w == NULL)
        {
            MessageBox(TEXT("给文件名分配暂存空间时出错!"), TEXT("错误信息"),MB_ICONERROR);
            return;
        }

        //把文件名拷贝到缓冲区
        DragQueryFile(hDropInfo,i,pPath_w,NameSize + 1);

        CString sPath_w(pPath_w);
        HeapFree(hHeap,HEAP_ZERO_MEMORY,pPath_w);

        CString sPathDir_w = sPath_w + "\\*.*";
        BOOL bWorking = fileChecker.FindFile(sPathDir_w);
        CStringArray sPathList;
        if (bWorking)
        {
            bWorking = fileChecker.FindNextFile();
            if (bWorking && fileChecker.IsDirectory())
            {
                Recurse(sPath_w,sPathList);
            }
        }
        else
        {
            sPathList.Add(sPath_w);
        }

        for (int lp = 0; lp < sPathList.GetSize(); lp++)
        {
            char *fpath = new char[(sPathList[lp].GetLength() + 1) * 2];
            int conv = WideCharToMultiByte(CP_ACP,0,sPathList[lp],sPathList[lp].GetLength(),fpath,
                                           (sPathList[lp].GetLength() + 1) * 2,NULL,NULL);
            fpath[conv] = '\0';

            if (m_fs.add_file(fpath))
            {
                m_DirList.AddString(GetFileName(sPathList[lp]));
                string str = "Create File ";;
                str += Wchar2Schar(GetFileName(sPathList[lp]));
                ShowInfo(str.c_str(),this);
            }
            delete []fpath;
        }
    }

    CDialog::OnDropFiles(hDropInfo);
	DrawBitMap(m_fs.get_bit_map());
}

void CFSShellDlg::OnBnClickedOk()
{
    int nCount = m_DirList.GetSelCount();
    if (nCount != 0)
    {
        int *CurSel = new int[nCount];
        m_DirList.GetSelItems(nCount,CurSel);
        for (int i = nCount - 1; i >= 0; i--)
        {
            CString name;
            m_DirList.GetText(CurSel[i],name);
            if (m_fs.delete_file(Wchar2Schar(name).c_str()))
            {
                m_DirList.DeleteString(CurSel[i]);
                string str = "Delete File ";;
                str += Wchar2Schar(name);
                ShowInfo(str.c_str(),this);
            }
        }
        DrawBitMap(m_fs.get_bit_map());
        delete []CurSel;
		ShowInfo("文件删除完毕!",this);
    }
	else
	{
		::AfxMessageBox(TEXT("必须选择至少一个文件!"));
	}
}

void CFSShellDlg::OnLbnSelchangeListDir()
{
    vector<COLOR> vGlobal = m_fs.get_bit_map();
    int nCount = m_DirList.GetSelCount();
    if (nCount != 0)
    {
        vector<COLOR> vColor = m_fs.get_bit_map();
        vColor = vector<COLOR>(vColor.size(),EMPTY);
        int *CurSel = new int[nCount];

        m_DirList.GetSelItems(nCount,CurSel);
        for (int i = 0; i < nCount; i++)
        {
            CString name;
            m_DirList.GetText(CurSel[i],name);
            vector<COLOR> t = m_fs.get_bit_map(Wchar2Schar(name).c_str());
            for (unsigned int z = 0; z < t.size(); z++)
            {
                vColor[z] = (COLOR)((int)vColor[z] | (int)t[z]);
            }
        }
        for (unsigned int i = 0; i < vGlobal.size(); i++)
        {
            if (vColor[i] > vGlobal[i])
                vGlobal[i] = vColor[i];
        }
    }
    DrawBitMap(vGlobal);
}

void CFSShellDlg::OnBnClickedCancel()
{
	CFileDialog dlg(FALSE,_T( "dsk" ), _T( "untitled_disk.dsk" ), 
		 OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T( "虚拟磁盘|*.dsk|所有文件|*||" ) );
	if (dlg.DoModal() == IDOK)
	{
		CString path = dlg.GetPathName();
		char *buf = new char[(path.GetLength() + 1) * 2];
		m_fs.dskmounted[m_fs.curr].save_to_file(Wchar2Schar(path).c_str());
		ShowInfo("虚拟磁盘已写入文件!",this);
	}
	
}

void CFSShellDlg::OnClose()
{
    exit(0);
}


void CFSShellDlg::OnLbnDblclkListDir()
{
	int nCount = m_DirList.GetSelCount();
	if (nCount != 0)
	{
		int *CurSel = new int[nCount];
		m_DirList.GetSelItems(nCount,CurSel);
		CString fname;
		m_DirList.GetText(CurSel[0],fname);
		unsigned int hFile;
		if(m_fs.dskmounted[m_fs.curr].open_file(Wchar2Schar(fname).c_str(),&hFile))
		{
			CFileShow fs(fname,hFile,&this->m_fs);
			delete []CurSel;
			fs.DoModal();
			m_fs.dskmounted[m_fs.curr].close_file(hFile);
		}
	}
}
