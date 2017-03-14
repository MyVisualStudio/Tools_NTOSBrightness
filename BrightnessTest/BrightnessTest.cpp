// BrightnessTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CScreenBrightness.h"
#include <iostream>
int main()
{
	DWORD dwBrightness, dwDimBrightness, dwStatusCount;
	BOOL bAdapt_Status;
	CBNotificationReceiverWithStatus* objCs;
	CScreenBrightness::Read(&dwBrightness);
	std::cout << "Current Brightness is: [" << dwBrightness << "] " << std::endl;
	CScreenBrightness::Read(&dwDimBrightness, CSystemPowerPlan::GetCurrent(), DIM_BRIGHTNESS);
	std::cout << "Current Dim Brightness is: [" << dwDimBrightness << "] " << std::endl;
	CScreenBrightness::Write((dwBrightness += 20, dwBrightness));
	std::cout << "Increased by 20." << std::endl << "Last Brightness is: [" << dwBrightness << "] " << std::endl;
	CScreenBrightness::Write(50, CSystemPowerPlan::GetCurrent(), DIM_BRIGHTNESS);
	CScreenBrightness::GetAdaptiveStatus(&bAdapt_Status);
	CScreenBrightness::SetAdaptiveStatus(!bAdapt_Status);
	std::cout << "Current AdaptiveStatus is: [" << bAdapt_Status << "] " << std::endl;
	system("pause");
	CBNotificationReceiver* pNotificationReceiver = new CBNotificationReceiver([](PVOID p) {
		std::cout << (char*)p << std::endl;
		CScreenBrightness::Write(100);
	}, (PVOID)"\n[Event Triggered]");
	CScreenBrightness::SetNotificationReceiver(pNotificationReceiver, 1);
	CScreenBrightness::GetNotificationReceiver(&objCs, &dwStatusCount);
	printf("NotificationReceiver changed successfully : 1\nAdded : %d\n", dwStatusCount);
	CScreenBrightness::SetNotificationReceiver(
		new CBNotificationReceiver([](PVOID p) {
		std::cout << (char*)p << std::endl;
	}, (PVOID)"\n[New NotificationReceiverFunction Enabled]"), 1);
	CScreenBrightness::GetNotificationReceiver(&objCs, &dwStatusCount);
	printf("NotificationReceiver changed successfully : 2\nAdded : %d\n", dwStatusCount);
	system("pause");
	return 0;
}

