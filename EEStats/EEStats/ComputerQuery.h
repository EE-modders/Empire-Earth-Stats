#pragma once

#include <string>
#include <winnt.h>
#include <windef.h>
#include <iostream>

class ComputerQuery
{

	// TODO: Maybe cache those info or cache the generated ua/header of matomo

public:

	ComputerQuery::ComputerQuery()
	{
		std::cout << "Wine: " << isWine() << std::endl;
		std::cout << "DirectX: " << getDirectX_MajorVersion() << std::endl;
		std::cout << "VM: " << runInVM() << std::endl;
		std::cout << "UID: " << getUID() << std::endl;
	};

	// Memory
	DWORDLONG getRAM();
	std::string getSimpleRAM(bool gonly = false);

	// GPU
	std::string getGraphicVendorId();
	std::string getGraphicDeviceId();
	std::string getGraphicName();
	std::string getGraphicVersion();
	std::string getGraphicCurrentRefreshRate();
	std::string getGraphicMaxRefreshRate();
	std::string getGraphicCurrentBitsPerPixel();
	std::string getGraphicDedicatedMemory();
	
	// CPU
	std::string getProcessorId();
	std::string getProcessorName();
	std::string getProcessorNumberOfCores();
	std::string getProcessorLoadPercentage();
	std::string getProcessorCurrentCorePercentage();

	// DX
	int getDirectX_MajorVersion();
	std::string getDirectX_WrapperVersion();

	// Screen
	SIZE getWindowsResolution();

	// Locale
	std::string getWindowsLocale();

	// Wine
	bool isWine();
	const char* getWineVersion();

	// VM
	bool runInVirtualPC();
	bool runInVMWare();
	bool runInVirtualBox();
	bool runInHyperviror();
	bool runInOtherVM();
	bool runInVM();

	// UID
	std::string getBiosSerial();

	std::string getUID()
	{
		return "dummy";
	}

	// Windows
	typedef enum WindowsVersion {
		Win11, Win10, Win8, Win8_1, Win7, WinVista, WinXP, WinUnknown
	} WindowsVersion;
	
	WindowsVersion getWindowsVersion();

};

