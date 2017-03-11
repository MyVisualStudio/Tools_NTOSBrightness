#include <Windows.h>
#include <PowrProf.h>
#include <exception>
#pragma comment(lib,"PowrProf.lib")

#ifndef _SCREEN_BRIGHTNESS_CTL
#define _SCREEN_BRIGHTNESS_CTL
typedef void (WINAPI *CNotifyProcedure)(PVOID pAttachment);
class CBrightnessNotify {
public:
	CBrightnessNotify(CNotifyProcedure pfnInitial, PVOID pAttachment);
	CBrightnessNotify(CNotifyProcedure pfnInitial);
	void SetAttachment(PVOID pAttachment);
	PVOID GetAttachment();
	void SetNotifyProcedure(CNotifyProcedure pfn);
	CNotifyProcedure GetNotifyProcedure();
	PVOID GetProcedureForApply();
	void SetRegistrationHandle(HPOWERNOTIFY hPowerNotify);
	HPOWERNOTIFY GetRegistrationHandle();
private:
	CBrightnessNotify();
	CNotifyProcedure lpfnNotifier = 0;
	PVOID pAttachment = 0, pRegHandle = 0;
};
enum POWER_TYPE
{
	POWER_TYPE_NULL = 255,
	AC_MODE = 1,
	DC_MODE = 0
};
typedef DWORD (WINAPI *pfnControllerRead)(_In_opt_ HKEY RootPowerKey,
	_In_ CONST GUID * SchemeGuid,
	_In_opt_ CONST GUID * SubGroupOfPowerSettingsGuid,
	_In_opt_ CONST GUID * PowerSettingGuid,
	_In_ LPDWORD AcValueIndex //Read:LPDWORD
	);
typedef DWORD(WINAPI *pfnControllerWrite)(_In_opt_ HKEY RootPowerKey,
	_In_ CONST GUID * SchemeGuid,
	_In_opt_ CONST GUID * SubGroupOfPowerSettingsGuid,
	_In_opt_ CONST GUID * PowerSettingGuid,
	_In_ DWORD AcValueIndex 
	);
class CSystemPowerPlan {
public:
	POWER_TYPE GetType();
	DWORD ReadValueIndex(CONST GUID * SubGroupOfPowerSettingsGuid,
		CONST GUID * PowerSettingGuid, LPDWORD AcValueIndex);
	DWORD WriteValueIndex(CONST GUID * SubGroupOfPowerSettingsGuid,
		CONST GUID * PowerSettingGuid, DWORD AcValueIndex);
	static CSystemPowerPlan* GetCurrent();
	static CSystemPowerPlan* GetByPowerType(POWER_TYPE nType);
private:
	CSystemPowerPlan() {};
	HRESULT Initialize(POWER_TYPE nType);
	pfnControllerRead pfnControlForRead;
	pfnControllerWrite pfnControlForWrite;
	POWER_TYPE nPowerType;
	GUID* pSchemeGuid;
};

enum BRIGHTNESS_STATE {
	NORMAL_BRIGHTNESS,
	DIM_BRIGHTNESS
};

class CScreenBrightness {
public:
	static HRESULT Read(_Out_ LPDWORD pResult);
	static HRESULT Read(_Out_ LPDWORD pResult, _In_ CSystemPowerPlan* refSysPowerPlan);
	static HRESULT Read(_Out_ LPDWORD pResult, _In_ CSystemPowerPlan* refSysPowerPlan, BRIGHTNESS_STATE dwState);

	static HRESULT Write(_In_ DWORD dwValue);
	static HRESULT Write(_In_ DWORD dwValue, _In_ CSystemPowerPlan* refSysPowerPlan);
	static HRESULT Write(_In_ DWORD dwValue, _In_ CSystemPowerPlan* refSysPowerPlan, BRIGHTNESS_STATE dwState);

	static HRESULT SetNotify(CBrightnessNotify* refNotify, BOOL bEnabled);
	static HRESULT GetNotify(CBrightnessNotify** refNotifyOut, LPBOOL lpblEnabled);

	static HRESULT SetAdaptiveStatus(BOOL blEnable);
	static HRESULT SetAdaptiveStatus(BOOL blEnable, _In_ CSystemPowerPlan* refSysPowerPlan);

	static HRESULT GetAdaptiveStatus(LPBOOL lpblEnable);
	static HRESULT GetAdaptiveStatus(LPBOOL lpblEnable, _In_ CSystemPowerPlan* refSysPowerPlan);
};

typedef DWORD(WINAPI *pfnPowerApplySettingChanges)(
	const GUID& SubGroupOfPowerSettingsGuid, const GUID& PowerSettingGuid);

#endif