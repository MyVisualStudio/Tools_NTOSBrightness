// BrightnessTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CScreenBrightness.h"
#include <iostream>
int main()
{
	DWORD dwBrightness;
	CScreenBrightness::Read(&dwBrightness);
	std::cout << "Current Brightness is: [" << dwBrightness << "] " << std::endl;
	CScreenBrightness::Write((dwBrightness += 20, dwBrightness));
	std::cout << "Increased by 20." << std::endl << "Current Brightness is: [" << dwBrightness << "] " << std::endl;
	system("pause");
	CBrightnessNotify* pNotify = new CBrightnessNotify([](PVOID p) {
		std::cout << (ULONG_PTR)p << std::endl;
	}, (PVOID)6666666);
	CScreenBrightness::SetNotify(pNotify, 1);
	system("pause");
	return 0;
}

