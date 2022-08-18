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
	ComputerQuery::ComputerQuery();

	// Memory
	float getRAM();
	std::string extremRound(double val);

	// GPU
	std::string getGraphicVendorId();
	std::string getGraphicDeviceId();
	std::string getGraphicName();
	std::string getGraphicVersion();
	uint32_t getGraphicCurrentRefreshRate();
	// std::string getGraphicMaxRefreshRate();
	uint32_t getGraphicCurrentBitsPerPixel();
	float getGraphicDedicatedMemory();
	
	// CPU
	std::string getProcessorId();
	std::string getProcessorName();
	std::string getProcessorArch();
	uint32_t getProcessorNumberOfCores();
	uint16_t getProcessorLoadPercentage();
	std::string getProcessorCurrentCorePercentage();

	// DX
	int getDirectX_MajorVersion();
	std::string getDirectX_WrapperVersion();
	std::string getDirectX_WrapperParams();

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

	WindowsVersion getWindowsVersionCQ();
	std::string getWindowsVersion();
	std::string getWindowsName();

private:
	std::unique_ptr<WmiHelper> _wmiHelper = std::make_unique<WmiHelper>();
	std::string _bestGraphicPNPDeviceID;
};

