#pragma once

#include <string>
#include <winnt.h>
#include <windef.h>

class ComputerQuery
{

	// TODO: Maybe cache those info or cache the generated ua/header of matomo

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
	std::string getGraphicMaxRefreshRate();
	std::string getGraphicCurrentBitsPerPixel();
	std::string getGraphicDedicatedMemory();
	
	// CPU
	std::string getProcessorId();
	std::string getProcessorName();
	std::string getProcessorNumberOfCores();
	std::string getProcessorLoadPercentage();
	std::string getProcessorCurrentCorePercentage();

	// Screen
	SIZE getWindowsResolution();

	// Locale
	std::string getWindowsLocale();

	// Wine
	bool isWine();
	const char* getWineVersion();

	// Windows
	typedef enum WindowsVersion {
		Win11, Win10, Win8, Win8_1, Win7, WinVista, WinXP, WinUnknown
	} WindowsVersion;
	
	WindowsVersion getWindowsVersion();

};

