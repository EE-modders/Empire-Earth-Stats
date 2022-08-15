#pragma once

#include <string>
#include <winnt.h>
#include <windef.h>
#include <iostream>

#include "WmiHelper.h"

class ComputerQuery
{

	// TODO: Maybe cache those info or cache the infos

public:
	// Memory
	DWORDLONG getRAM();
	std::string getSimpleRAM(bool gonly = false);

	// GPU
	std::string getGraphicVendorId();
	std::string getGraphicDeviceId();
	std::string getGraphicName();
	std::string getGraphicVersion();
	std::string getGraphicCurrentRefreshRate();
	// std::string getGraphicMaxRefreshRate();
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
	bool runInOtherVM();
	bool runInVM();

	// UID
	std::string getBiosSerial();
	std::string getComputerSerial();
	std::string getWindowsDiskSerial();
	std::string getUID();

	// Info
	void printInfos();

	// Windows
	typedef enum WindowsVersion {
		Win11, Win10, Win8, Win8_1, Win7, WinVista, WinXP, WinUnknown
	} WindowsVersion;
	
	WindowsVersion getWindowsVersion();

private:
	std::unique_ptr<WmiHelper> _wmiHelper = std::make_unique<WmiHelper>();

};

