
// ScreenBrightnessDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScreenBrightness.h"
#include "ScreenBrightnessDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CScreenBrightnessDlg 对话框



CScreenBrightnessDlg::CScreenBrightnessDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SCREENBRIGHTNESS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CScreenBrightnessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER1, Trackbar_sb);
}

BEGIN_MESSAGE_MAP(CScreenBrightnessDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CREATE()
//	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, &CScreenBrightnessDlg::OnNMCustomdrawSlider1)
//ON_NOTIFY(TRBN_THUMBPOSCHANGING, IDC_SLIDER1, &CScreenBrightnessDlg::OnTRBNThumbPosChangingSlider1)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, &CScreenBrightnessDlg::OnNMCustomdrawSlider1)
END_MESSAGE_MAP()


// CScreenBrightnessDlg 消息处理程序

BOOL CScreenBrightnessDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CScreenBrightnessDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CScreenBrightnessDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CScreenBrightnessDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//DEVMODE pDev;


#include <PowrProf.h>
#pragma comment(lib,"PowrProf.lib")
enum POWER_TYPE
{
	POWER_TYPE_NULL,
	AC_MODE,
	DC_MODE
};
 DWORD WINAPI PowerApplySettingChanges(const GUID& settingsA, const GUID& settingsB)
 {
	 typedef DWORD( WINAPI *PowerApplySettingChangesProc)(const GUID& settingsA, const GUID& settingsB);
	 PowerApplySettingChangesProc _p;
	 _p = (PowerApplySettingChangesProc)GetProcAddress(GetModuleHandle(L"PowrProf.dll"), "PowerApplySettingChanges");
	 return _p(settingsA,  settingsB);
 }
 POWER_TYPE GetCurrentPowerType()
 {
	 SYSTEM_POWER_STATUS pSysPower;
	 if (GetSystemPowerStatus(&pSysPower))
	 {
		 return pSysPower.ACLineStatus == 1 ? AC_MODE : DC_MODE;
	 }
	 return POWER_TYPE_NULL;
 }
 BOOL NotifyCallbackEnabled = TRUE;
DWORD SetBrightnessValue(POWER_TYPE powerType, DWORD brightness)
{
	GUID* SchemeGuid;
	DWORD result = PowerGetActiveScheme(0, &SchemeGuid);
	if (result != ERROR_SUCCESS) return result;
	if (powerType == AC_MODE)
	{
		result = PowerWriteACValueIndex(0, SchemeGuid, &GUID_VIDEO_SUBGROUP, &GUID_DEVICE_POWER_POLICY_VIDEO_BRIGHTNESS, brightness);
	}
	else if (powerType == DC_MODE)
	{
		result = PowerWriteDCValueIndex(0, SchemeGuid, &GUID_VIDEO_SUBGROUP, &GUID_DEVICE_POWER_POLICY_VIDEO_BRIGHTNESS, brightness);
	}
	else
	{
		return ERROR_INVALID_VARIANT;
	}

	if (result != ERROR_SUCCESS) return result;
	return PowerApplySettingChanges(GUID_VIDEO_SUBGROUP, GUID_DEVICE_POWER_POLICY_VIDEO_BRIGHTNESS);
}
DWORD GetBrightnessValue(POWER_TYPE powerType,DWORD* brightness)
{
	GUID* SchemeGuid;
	DWORD result = PowerGetActiveScheme(0, &SchemeGuid);
	if (result != ERROR_SUCCESS) return result;
	if (powerType==AC_MODE)
	{
     result = PowerReadACValueIndex(0, SchemeGuid, &GUID_VIDEO_SUBGROUP, &GUID_DEVICE_POWER_POLICY_VIDEO_BRIGHTNESS, brightness);
	}
	else if(powerType == DC_MODE)
	{
    result = PowerReadDCValueIndex(0, SchemeGuid, &GUID_VIDEO_SUBGROUP, &GUID_DEVICE_POWER_POLICY_VIDEO_BRIGHTNESS, brightness);
	}
	else
	{
		return ERROR_INVALID_VARIANT;
	}
	
	if (result != ERROR_SUCCESS) return result;
	return PowerApplySettingChanges(GUID_VIDEO_SUBGROUP, GUID_DEVICE_POWER_POLICY_VIDEO_BRIGHTNESS);
}
CScreenBrightnessDlg* m_instance;
DWORD  WINAPI BrightnessNotifyCallback(PVOID,PVOID,PVOID)
{
	DWORD pos;
	if (NotifyCallbackEnabled)
	{
	GetBrightnessValue(GetCurrentPowerType(),&pos);
	m_instance->Trackbar_sb.SetPos(pos);
	}
	return 0;
}
HPOWERNOTIFY RegistrationHandle;
DWORD SetNotificationPort(BOOL  Enabled=TRUE)
{
	HANDLE Recipient= BrightnessNotifyCallback;
	DWORD result=-1;
	if (Enabled)
	{
		result = PowerSettingRegisterNotification(
		&GUID_DEVICE_POWER_POLICY_VIDEO_BRIGHTNESS,
		2u,
		&Recipient,
		&RegistrationHandle);
	}
	else
	{
		 result = PowerSettingUnregisterNotification(
			RegistrationHandle);
	}
	return result;
}
DWORD lastData=-1;

void CScreenBrightnessDlg::OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	UpdateData(TRUE);
	Trackbar_sb.SetRangeMax(100, 0);
	if (lastData == Trackbar_sb.GetPos())
	{
		goto final_;
	}
	lastData = Trackbar_sb.GetPos();
	NotifyCallbackEnabled = FALSE;
	SetBrightnessValue(GetCurrentPowerType(),Trackbar_sb.GetPos());
	NotifyCallbackEnabled = TRUE;
	final_:
	UpdateData(FALSE);
	*pResult = 0;
}

int CScreenBrightnessDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;
	/*
	if (!EnumDisplaySettingsExW(0, 0xFFFFFFFF, &pDev, 6))
	{
	MessageBox(L"Error on EnumDisplaySettingsExW", L"Error", MB_ICONWARNING);
	exit(0);
	}*/
	m_instance = this;
	SetNotificationPort();

	// TODO:  在此添加您专用的创建代码

	return 0;
}