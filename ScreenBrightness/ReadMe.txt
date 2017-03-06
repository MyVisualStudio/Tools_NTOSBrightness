最新版本（2017）请看项目BrightnessTest，写的更清楚

--------------------------------------------------------------
关键代码：

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
