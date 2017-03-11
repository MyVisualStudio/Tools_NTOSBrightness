#include "CScreenBrightness.h"

#define With_CSystemPowerPlan_CheckParam(RoutineName,Parameter) \
CSystemPowerPlan* pPowerPlan = CSystemPowerPlan::GetCurrent(); \
if (!pPowerPlan) throw new std::exception("Unsupported Platform"); \
HRESULT hResult = RoutineName((Parameter), pPowerPlan);\
delete pPowerPlan; \
return hResult;

CBrightnessNotify* pNotifyInst;
BOOL _bEnabled;

POWER_TYPE CSystemPowerPlan::GetType() {
	return nPowerType;
}

DWORD CSystemPowerPlan::ReadValueIndex(CONST GUID * SubGroupOfPowerSettingsGuid, 
	CONST GUID * PowerSettingGuid, LPDWORD AcValueIndex)
{
	return pfnControlForRead(0, pSchemeGuid, SubGroupOfPowerSettingsGuid, 
		PowerSettingGuid, AcValueIndex);
}
DWORD WINAPI PowerApplySettingChanges(
	const GUID& SubGroupOfPowerSettingsGuid, const GUID& PowerSettingGuid)
{
	return ((pfnPowerApplySettingChanges)GetProcAddress(
		GetModuleHandleW(L"PowrProf.dll"), "PowerApplySettingChanges"))
		(SubGroupOfPowerSettingsGuid, PowerSettingGuid);
}
DWORD CSystemPowerPlan::WriteValueIndex(CONST GUID * SubGroupOfPowerSettingsGuid, 
	CONST GUID * PowerSettingGuid, DWORD AcValueIndex)
{
	DWORD dwStatus;
	if (dwStatus = pfnControlForWrite(0, pSchemeGuid, SubGroupOfPowerSettingsGuid,
		PowerSettingGuid, AcValueIndex), dwStatus != 0)
		return dwStatus;
	return PowerApplySettingChanges(*SubGroupOfPowerSettingsGuid,
		*PowerSettingGuid);
}

POWER_TYPE GetCurrentPowerType()
{
	SYSTEM_POWER_STATUS stSysPower;
	if (GetSystemPowerStatus(&stSysPower))
		return (POWER_TYPE)stSysPower.ACLineStatus;
	return POWER_TYPE_NULL;
}

CSystemPowerPlan* CSystemPowerPlan::GetCurrent() {
	return GetByPowerType(GetCurrentPowerType());
}

CSystemPowerPlan * CSystemPowerPlan::GetByPowerType(POWER_TYPE nType)
{
	CSystemPowerPlan* pResult = new CSystemPowerPlan();
	if (!SUCCEEDED(pResult->Initialize(nType))) {
		delete pResult;
		return 0;
	}
	return pResult;
}



HRESULT CSystemPowerPlan::Initialize(POWER_TYPE nType)
{
	nPowerType = nType;
	switch (nType)
	{
	case AC_MODE: {
		pfnControlForRead = PowerReadACValueIndex;
		pfnControlForWrite = PowerWriteACValueIndex;
		break;
	}
	case DC_MODE: {
		pfnControlForRead = PowerReadDCValueIndex;
		pfnControlForWrite = PowerWriteDCValueIndex;
		break;
	}
	default:
		return E_FAIL;
	}
	return (PowerGetActiveScheme(0, &pSchemeGuid) == ERROR_SUCCESS) ? S_OK : E_FAIL;
}

CBrightnessNotify::CBrightnessNotify(CNotifyProcedure pfnInitial, PVOID pAttachment)
{
	SetNotifyProcedure(pfnInitial);
	SetAttachment(pAttachment);
}

CBrightnessNotify::CBrightnessNotify(CNotifyProcedure pfnInitial) {
	CBrightnessNotify(pfnInitial, 0);
}
void CBrightnessNotify::SetAttachment(PVOID pAttachment) {
	this->pAttachment = pAttachment;
}
PVOID CBrightnessNotify::GetAttachment() {
	return pAttachment;
}
void CBrightnessNotify::SetNotifyProcedure(CNotifyProcedure pfn) {
	lpfnNotifier = pfn;
}
CNotifyProcedure CBrightnessNotify::GetNotifyProcedure() {
	return lpfnNotifier;
}

#if defined(_M_X64) || defined(__x86_64__)
PVOID CBrightnessNotify::GetProcedureForApply() {

}
#else
//32Bit Details
#pragma pack(push, 1)
struct ASMInstruction {
	UINT8 uInstruction;
	UINT_PTR uParameter;
};
#pragma pack(pop)
PVOID CBrightnessNotify::GetProcedureForApply() {
	/*
	ASM Code like this:
	push    [Context]
	mov     eax, [NotifyProcedure]
	call    eax
	xor     eax, eax
	retn    12
	*/
	size_t dwFuncLength = sizeof(ASMInstruction) * 2 + sizeof(INT) + 3;
	PVOID pAllocated = VirtualAlloc(0, dwFuncLength, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	*(ASMInstruction*)pAllocated = { 0x68, (UINT_PTR)pAttachment };
	//push pAttachment
	*(ASMInstruction*)((ULONG_PTR)pAllocated + sizeof(ASMInstruction)) = { 0xB8,(UINT_PTR)lpfnNotifier };
	//mov eax,lpfnNotifier
	*(CHAR*)((ULONG_PTR)pAllocated + sizeof(ASMInstruction) * 2) = 0xFF;//call
	*(CHAR*)((ULONG_PTR)pAllocated + sizeof(ASMInstruction) * 2 + 1) = 0xD0;//eax
	*(CHAR*)((ULONG_PTR)pAllocated + sizeof(ASMInstruction) * 2 + 2) = 0x31;//xor
	*(INT*)((ULONG_PTR)pAllocated + sizeof(ASMInstruction) * 2 + 3) = 0xC0;//eax,eax

	*(CHAR*)((ULONG_PTR)pAllocated + sizeof(ASMInstruction) * 2 + sizeof(INT)) = 0xC2;//retn
	*(INT*)((ULONG_PTR)pAllocated + sizeof(ASMInstruction) * 2 + sizeof(INT) + 1) = 0xC;//12
	*(CHAR*)((ULONG_PTR)pAllocated + sizeof(ASMInstruction) * 2 + sizeof(INT) + 3) = 0xCC;//int 3
	return pAllocated;
}
#endif
void CBrightnessNotify::SetRegistrationHandle(HPOWERNOTIFY hPowerNotify) {
	pRegHandle = hPowerNotify;
}
HPOWERNOTIFY CBrightnessNotify::GetRegistrationHandle() {
	return pRegHandle;
}


HRESULT CScreenBrightness::Read(LPDWORD pResult) {
	With_CSystemPowerPlan_CheckParam(Read, pResult);
}
HRESULT CScreenBrightness::Read(LPDWORD pResult, CSystemPowerPlan * refSysPowerPlan) {
	return Read(pResult, refSysPowerPlan, NORMAL_BRIGHTNESS);
}
HRESULT CScreenBrightness::Read(LPDWORD pResult, CSystemPowerPlan * refSysPowerPlan, BRIGHTNESS_STATE dwState)
{
	const GUID* pGUIDSetting;
	switch (dwState)
	{
	case NORMAL_BRIGHTNESS: {
		pGUIDSetting = &GUID_DEVICE_POWER_POLICY_VIDEO_BRIGHTNESS;
		break;
	}
	case DIM_BRIGHTNESS: {
		pGUIDSetting = &GUID_DEVICE_POWER_POLICY_VIDEO_DIM_BRIGHTNESS;
		break;
	}
	default:
		return E_FAIL;
	}
	return (refSysPowerPlan->ReadValueIndex(
		&GUID_VIDEO_SUBGROUP, pGUIDSetting, pResult) == ERROR_SUCCESS) ? S_OK : E_FAIL;
}
HRESULT CScreenBrightness::Write(DWORD dwValue) {
	With_CSystemPowerPlan_CheckParam(Write, dwValue);
}

HRESULT CScreenBrightness::Write(DWORD dwValue, CSystemPowerPlan * refSysPowerPlan) {
	return Write(dwValue, refSysPowerPlan, NORMAL_BRIGHTNESS);
}

HRESULT CScreenBrightness::Write(DWORD dwValue, CSystemPowerPlan * refSysPowerPlan, BRIGHTNESS_STATE dwState)
{
	const GUID* pGUIDSetting;
	switch (dwState)
	{
	case NORMAL_BRIGHTNESS: {
		pGUIDSetting = &GUID_DEVICE_POWER_POLICY_VIDEO_BRIGHTNESS;
		break;
	}
	case DIM_BRIGHTNESS: {
		pGUIDSetting = &GUID_DEVICE_POWER_POLICY_VIDEO_DIM_BRIGHTNESS;
		break;
	}
	default:
		return E_FAIL;
	}
	return (refSysPowerPlan->WriteValueIndex(
		&GUID_VIDEO_SUBGROUP, pGUIDSetting, dwValue) == ERROR_SUCCESS) ? S_OK : E_FAIL;
}

HRESULT CScreenBrightness::SetNotify(CBrightnessNotify * refNotify, BOOL bEnabled)
{
	HANDLE hRecipient = refNotify->GetProcedureForApply();
	pNotifyInst = refNotify;
	_bEnabled = bEnabled;
	if (bEnabled) {
		HANDLE hRegistrationHandle;
		HRESULT hReturn = (PowerSettingRegisterNotification(
			&GUID_DEVICE_POWER_POLICY_VIDEO_BRIGHTNESS,
			DEVICE_NOTIFY_CALLBACK,
			&hRecipient,
			&hRegistrationHandle) == ERROR_SUCCESS) ? S_OK : E_FAIL;
		refNotify->SetRegistrationHandle(hRegistrationHandle);
		return hReturn;
	}
	if (PowerSettingUnregisterNotification(refNotify->GetRegistrationHandle()) != ERROR_SUCCESS)
		return E_FAIL;
	refNotify->SetRegistrationHandle(0);
	return S_OK;
}

HRESULT CScreenBrightness::GetNotify(CBrightnessNotify ** refNotifyOut, LPBOOL lpblEnabled) {
	*refNotifyOut = pNotifyInst;
	*lpblEnabled = _bEnabled;
	return S_OK;
}

HRESULT CScreenBrightness::SetAdaptiveStatus(BOOL blEnable) {
	With_CSystemPowerPlan_CheckParam(SetAdaptiveStatus, blEnable);
}

HRESULT CScreenBrightness::SetAdaptiveStatus(BOOL blEnable, CSystemPowerPlan * refSysPowerPlan) {
	return (refSysPowerPlan->WriteValueIndex(
		&GUID_VIDEO_SUBGROUP, &GUID_VIDEO_ADAPTIVE_DISPLAY_BRIGHTNESS, blEnable) == ERROR_SUCCESS) ? S_OK : E_FAIL;
}

HRESULT CScreenBrightness::GetAdaptiveStatus(LPBOOL lpblEnable) {
	With_CSystemPowerPlan_CheckParam(GetAdaptiveStatus, lpblEnable);
}

HRESULT CScreenBrightness::GetAdaptiveStatus(LPBOOL lpblEnable, CSystemPowerPlan * refSysPowerPlan) {
	return (refSysPowerPlan->ReadValueIndex(
		&GUID_VIDEO_SUBGROUP, &GUID_VIDEO_ADAPTIVE_DISPLAY_BRIGHTNESS, 
		(LPDWORD)lpblEnable) == ERROR_SUCCESS) ? S_OK : E_FAIL;
}
