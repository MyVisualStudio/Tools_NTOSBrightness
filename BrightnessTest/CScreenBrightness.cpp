#include "stdafx.h"
#include "CScreenBrightness.h"

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
	typedef DWORD(WINAPI *pfnPowerApplySettingChanges)(
		const GUID& SubGroupOfPowerSettingsGuid, const GUID& PowerSettingGuid);
	pfnPowerApplySettingChanges pfnTarget;
	pfnTarget = (pfnPowerApplySettingChanges)GetProcAddress(
		GetModuleHandleW(L"PowrProf.dll"), "PowerApplySettingChanges");
	return pfnTarget(SubGroupOfPowerSettingsGuid, PowerSettingGuid);
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
	SYSTEM_POWER_STATUS pSysPower;
	if (GetSystemPowerStatus(&pSysPower))
		return pSysPower.ACLineStatus == 1 ? AC_MODE : DC_MODE;
	return POWER_TYPE_NULL;
}

CSystemPowerPlan* CSystemPowerPlan::GetCurrent()
{
	return GetByPowerType(GetCurrentPowerType());
}

CSystemPowerPlan * CSystemPowerPlan::GetByPowerType(POWER_TYPE nType)
{
	CSystemPowerPlan* pResult = new CSystemPowerPlan();
	if (!SUCCEEDED(pResult->Initialize(nType)))
		return 0;
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
DWORD WINAPI PureProcedureTemplate(PVOID,PVOID,PVOID) {
	ULONG_PTR pContext = 666666;
	CNotifyProcedure pDestination = (CNotifyProcedure)233333;
	pDestination((PVOID)pContext);
	return 0;
}
void ReplacePatternInTemplate(PVOID pfn, size_t dwLength, ULONG_PTR ulSrc, ULONG_PTR ulReplace) {
	for (ULONG_PTR i = (ULONG_PTR)pfn; i < dwLength; i += sizeof(ULONG_PTR)) 
		if ((*(ULONG_PTR*)i) == ulSrc) (*(ULONG_PTR*)i) = ulReplace;
}
PVOID CBrightnessNotify::GetProcedureForApply() {
	size_t dwTotalLen = (size_t)ReplacePatternInTemplate - (size_t)PureProcedureTemplate;
	PVOID pAllocatedCodeStore = VirtualAlloc(0, dwTotalLen, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	memcpy(pAllocatedCodeStore, PureProcedureTemplate, dwTotalLen);
	ReplacePatternInTemplate(pAllocatedCodeStore, dwTotalLen, 233333, (ULONG_PTR)lpfnNotifier);
	ReplacePatternInTemplate(pAllocatedCodeStore, dwTotalLen, 666666, (ULONG_PTR)pAttachment);
	return pAllocatedCodeStore;
}

void CBrightnessNotify::SetRegistrationHandle(HPOWERNOTIFY hPowerNotify) {
	pRegHandle = hPowerNotify;
}
HPOWERNOTIFY CBrightnessNotify::GetRegistrationHandle() {
	return pRegHandle;
}

HRESULT CScreenBrightness::Read(LPDWORD pResult) {
	return Read(pResult, CSystemPowerPlan::GetCurrent());
}
HRESULT CScreenBrightness::Read(LPDWORD pResult, CSystemPowerPlan * refSysPowerPlan)
{
	return (refSysPowerPlan->ReadValueIndex(&GUID_VIDEO_SUBGROUP, 
		&GUID_DEVICE_POWER_POLICY_VIDEO_BRIGHTNESS, pResult) == ERROR_SUCCESS) ? S_OK : E_FAIL;
}
HRESULT CScreenBrightness::Write(DWORD dwValue) {
	return Write(dwValue,CSystemPowerPlan::GetCurrent());
}

HRESULT CScreenBrightness::Write(DWORD dwValue, CSystemPowerPlan * refSysPowerPlan)
{
	refSysPowerPlan->WriteValueIndex(&GUID_VIDEO_SUBGROUP,
		&GUID_DEVICE_POWER_POLICY_VIDEO_BRIGHTNESS, dwValue);

	return E_NOTIMPL;
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

HRESULT CScreenBrightness::GetNotify(CBrightnessNotify ** refNotifyOut, LPBOOL lpblEnabled)
{
	*refNotifyOut = pNotifyInst;
	*lpblEnabled = _bEnabled;
	return S_OK;
}
