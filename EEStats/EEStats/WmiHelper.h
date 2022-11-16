#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <Wbemidl.h>
#include <atlcomcli.h>

class WmiHelper
{
public:
	WmiHelper();
	~WmiHelper();

	std::vector<std::wstring> queryWMI(const char* query, LPCWSTR value);
	std::unordered_multimap<std::wstring, std::wstring> queryKeyValWMI(const char* query, LPCWSTR asKey, LPCWSTR asValue);

private:
	bool _init = false;
	// CComPtr<...> is better but crash...
	IWbemLocator* _pLoc;
	IWbemServices* _pSvc;
};