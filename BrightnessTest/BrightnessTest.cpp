// BrightnessTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CScreenBrightness.h"
#include <iostream>
int main()
{
	DWORD dwBrightness, dwDimBrightness;
	BOOL bAdapt_Status;
	CScreenBrightness::Read(&dwBrightness);
	std::cout << "Current Brightness is: [" << dwBrightness << "] " << std::endl;
	CScreenBrightness::Read(&dwDimBrightness, CSystemPowerPlan::GetCurrent(), DIM_BRIGHTNESS);
	std::cout << "Current Dim Brightness is: [" << dwDimBrightness << "] " << std::endl;
	CScreenBrightness::Write((dwBrightness += 20, dwBrightness));
	std::cout << "Increased by 20." << std::endl << "Current Brightness is: [" << dwBrightness << "] " << std::endl;
	CScreenBrightness::Write(50, CSystemPowerPlan::GetCurrent(), DIM_BRIGHTNESS);
	CScreenBrightness::GetAdaptiveStatus(&bAdapt_Status);
	CScreenBrightness::SetAdaptiveStatus(!bAdapt_Status);
	std::cout << "Current AdaptiveStatus is: [" << bAdapt_Status << "] " << std::endl;
	system("pause");
	CBrightnessNotify* pNotify = new CBrightnessNotify([](PVOID p) {
		std::cout << (ULONG_PTR)p << std::endl;
		CScreenBrightness::Write(100);
	}, (PVOID)6666666);
	CScreenBrightness::SetNotify(pNotify, 1);
	system("pause");
	return 0;
}

