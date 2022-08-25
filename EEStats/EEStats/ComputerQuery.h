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
	ComputerQuery();

	// Memory
	float getRAM();

	// GPU
	std::string getGraphicVendorId();
	std::string getGraphicDeviceId();
	std::string getGraphicName();
	std::string getGraphicVersion();
	uint32_t getGraphicRefreshRate();
	// std::string getGraphicMaxRefreshRate();
	uint32_t getGraphicBitsPerPixel();
	float getGraphicDedicatedMemory();
	
	// CPU
	std::string getProcessorId();
	std::string getProcessorName();
	std::string getProcessorArch();
	uint32_t getProcessorNumberOfCores();
	uint16_t getProcessorLoadPercentage(); // all cores

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

	// Windows (Those nb allow > & < op)
	typedef enum WindowsVersion {
		Win11 = 700, Win10 = 600, Win8 = 500, Win8_1 = 400, Win7 = 300, WinVista = 200, WinXP = 100, WinUnknown = -1
	} WindowsVersion;

	WindowsVersion getWindowsVersionCQ();
	std::string getWindowsVersion();
	std::string getWindowsName();

private:
	std::unique_ptr<WmiHelper> _wmiHelper = std::make_unique<WmiHelper>();
	std::string _bestGraphicPNPDeviceID;
	SIZE _windowsResolution;
	unsigned int _refreshRate;
	unsigned int _bitsPerPixel;
};

