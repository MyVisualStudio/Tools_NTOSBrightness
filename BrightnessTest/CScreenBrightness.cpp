#include "CScreenBrightness.h"

#define With_CSystemPowerPlan_CheckParam(RoutineName,Parameter) \
CSystemPowerPlan* pPowerPlan = CSystemPowerPlan::GetCurrent(); \
if (!pPowerPlan) throw new std::exception("Unsupported Platform"); \
HRESULT hResult = RoutineName((Parameter), pPowerPlan);\
delete pPowerPlan; \
return hResult;

/*
CBNotificationReceiver* pNotificationReceiverInst;
BOOL _bEnabled;
*/

std::vector<CBNotificationReceiverWithStatus> vectNotificationReceiver;
std::map<CBNotificationReceiver*, size_t>  mapStatusLocation;

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

CBNotificationReceiver::CBNotificationReceiver(CNotificationReceiverProcedure pfnInitial, PVOID pAttachment)
{
	SetNotificationReceiverProcedure(pfnInitial);
	SetAttachment(pAttachment);
}

CBNotificationReceiver::CBNotificationReceiver(CNotificationReceiverProcedure pfnInitial) {
	CBNotificationReceiver(pfnInitial, 0);
}
void CBNotificationReceiver::SetAttachment(PVOID pAttachment) {
	this->pAttachment = pAttachment;
}
PVOID CBNotificationReceiver::GetAttachment() {
	return pAttachment;
}
void CBNotificationReceiver::SetNotificationReceiverProcedure(CNotificationReceiverProcedure pfn) {
	lpfnNotificationReceiver = pfn;
}
CNotificationReceiverProcedure CBNotificationReceiver::GetNotificationReceiverProcedure() {
	return lpfnNotificationReceiver;
}

#if defined(_M_X64) || defined(__x86_64__)
PVOID CBNotificationReceiver::GetProcedureForApply() {
	size_t dwFuncLength = 10000;
	PVOID pAllocated = VirtualAlloc(0, dwFuncLength, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	/*
	Because of x64-platform CppCompiler's policy(stdcall CONVERT TO cdecl)
	sub     rsp, 28h
	mov     rcx, 18D5D42AAh
	mov     rax, 56EC66955h
	call    rax
	xor     eax, eax
	add     rsp, 28h
	retn
	48 83  EC 28 48 B9 AA 42 5D 8D 01 00 00 00 48 
	B8 55 69  C6 6E 05 00 00 00 FF D0 33 C0 48 83 
	C4 28 C3
    */
	
}
#else
//32Bit Details
#pragma pack(push, 1)
struct ASMInstruction {
	UINT8 uInstruction;
	UINT_PTR uParameter;
};
#pragma pack(pop)
PVOID CBNotificationReceiver::GetProcedureForApply() {
	/*
	ASM Code like this:
	push    [Context]
	mov     eax, [NotificationReceiverProcedure]
	call    eax
	xor     eax, eax
	retn    12
	*/
	size_t dwFuncLength = sizeof(ASMInstruction) * 2 + sizeof(INT) + 3;
	PVOID pAllocated = VirtualAlloc(0, dwFuncLength, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	*(ASMInstruction*)pAllocated = { 0x68, (UINT_PTR)pAttachment };
	//push pAttachment
	*(ASMInstruction*)((ULONG_PTR)pAllocated + sizeof(ASMInstruction)) = { 0xB8,(UINT_PTR)lpfnNotificationReceiver };
	//mov eax,lpfnNotificationReceiver
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
void CBNotificationReceiver::SetRegistrationHandle(HPOWERNOTIFY hPowerNotificationReceiver) {
	pRegHandle = hPowerNotificationReceiver;
}
HPOWERNOTIFY CBNotificationReceiver::GetRegistrationHandle() {
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

HRESULT CScreenBrightness::SetNotificationReceiver(CBNotificationReceiver * refNotificationReceiver, BOOL bEnabled)
{
	HANDLE hRecipient = refNotificationReceiver->GetProcedureForApply();
	CBNotificationReceiverWithStatus objStatus = { refNotificationReceiver ,bEnabled };
	//pNotificationReceiverInst = refNotificationReceiver;
	//_bEnabled = bEnabled;
	if (mapStatusLocation[refNotificationReceiver] == 0) {
		vectNotificationReceiver.push_back(objStatus);
		mapStatusLocation[refNotificationReceiver] = vectNotificationReceiver.size();
	}
	else 
		vectNotificationReceiver[mapStatusLocation[refNotificationReceiver] - 1].bEnabled = bEnabled;

	if (bEnabled) {
		HANDLE hRegistrationHandle;
		HRESULT hReturn = (PowerSettingRegisterNotification(
			&GUID_DEVICE_POWER_POLICY_VIDEO_BRIGHTNESS,
			DEVICE_NOTIFY_CALLBACK,
			&hRecipient,
			&hRegistrationHandle) == ERROR_SUCCESS) ? S_OK : E_FAIL;
		refNotificationReceiver->SetRegistrationHandle(hRegistrationHandle);
		return hReturn;
	}
	if (PowerSettingUnregisterNotification(refNotificationReceiver->GetRegistrationHandle()) != ERROR_SUCCESS)
		return E_FAIL;
	refNotificationReceiver->SetRegistrationHandle(0);
	return S_OK;
}

HRESULT CScreenBrightness::GetNotificationReceiver(CBNotificationReceiverWithStatus** pObjColl, LPDWORD lpdwLength) {
	*pObjColl = CreateArrayFromStdVector(vectNotificationReceiver);
	*lpdwLength = vectNotificationReceiver.size();
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
