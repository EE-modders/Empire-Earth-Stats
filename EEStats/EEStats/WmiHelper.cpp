#include "pch.h"

#include "WmiHelper.h"

#include <comdef.h>
#include <iostream>
#include <string>

#pragma comment(lib, "wbemuuid.lib")

WmiHelper::WmiHelper()
{
    HRESULT hres;
    // Step 1: --------------------------------------------------
    // Initialize COM. ------------------------------------------

    _init = false;
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
    {
        std::cout << "Failed to initialize COM library. Error code = 0x"
            << std::hex << hres << std::endl;
        return;                  // Program has failed.
    }

    // Step 2: --------------------------------------------------
    // Set general COM security levels --------------------------

    hres = CoInitializeSecurity(
        NULL,
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities 
        NULL                         // Reserved
    );


    if (hres != RPC_E_TOO_LATE && FAILED(hres))
    {
        std::cout << "Failed to initialize security. Error code = 0x"
            << std::hex << hres << std::endl;
        CoUninitialize();
        return;                    // Program has failed.
    }

    // Step 3: ---------------------------------------------------
    // Obtain the initial locator to WMI -------------------------

    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&_pLoc);

    if (FAILED(hres))
    {
        std::cout << "Failed to create IWbemLocator object."
            << " Err code = 0x"
            << std::hex << hres << std::endl;
        CoUninitialize();
        return;                 // Program has failed.
    }

    // Step 4: -----------------------------------------------------
    // Connect to WMI through the IWbemLocator::ConnectServer method

    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hres = _pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
        NULL,                    // User name. NULL = current user
        NULL,                    // User password. NULL = current
        0,                       // Locale. NULL indicates current
        NULL,                    // Security flags.
        0,                       // Authority (for example, Kerberos)
        0,                       // Context object 
        &_pSvc                    // pointer to IWbemServices proxy
    );

    if (FAILED(hres))
    {
        std::cout << "Could not connect. Error code = 0x"
            << std::hex << hres << std::endl;
        CoUninitialize();
        return;                // Program has failed.
    }


    // Step 5: --------------------------------------------------
    // Set security levels on the proxy -------------------------

    hres = CoSetProxyBlanket(
        _pSvc,                        // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // Server principal name 
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities 
    );

    if (FAILED(hres))
    {
        std::cout << "Could not set proxy blanket. Error code = 0x"
            << std::hex << hres << std::endl;
        CoUninitialize();
        return;               // Program has failed.
    }
    _init = true;
}

WmiHelper::~WmiHelper()
{
    if (_init) {
        /*
        *
        * I tried everything... I just can't understand why this keep crashing instant
        * Storing them as ptr and release them like that don' work
        * CComPtr also don't work (but not recommanded since CoUninitialize should be call after the Release)
        *
        *
        if (_pSvc)
            _pSvc->Release();
        if (_pLoc)
            _pLoc->Release();
        *
        */
        CoUninitialize();
    }
}

std::vector<std::wstring> WmiHelper::queryWMI(const char* query, LPCWSTR value)
{
    std::vector<std::wstring> result;
    HRESULT hres;

    if (!_init)
        return result;

    // Step 6: --------------------------------------------------
    // Use the IWbemServices pointer to make requests of WMI ----

    // For example, get the name of the operating system
    IEnumWbemClassObject* pEnumerator = NULL;
    hres = _pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t(query),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres))
    {
        std::cout << "Query for operating system name failed."
            << " Error code = 0x"
            << std::hex << hres << std::endl;
        return result;               // Program has failed.
    }

    // Step 7: -------------------------------------------------
    // Get the data from the query in step 6 -------------------

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
            &pclsObj, &uReturn);

        if (0 == uReturn || hr != S_OK)
        {
            break;
        }

        VARIANT vtProp;

        VariantInit(&vtProp);

        // Get the value of the Name property
        hr = pclsObj->Get(value, 0, &vtProp, 0, 0);
        if (hr == S_OK && vtProp.vt == VT_BSTR && vtProp.bstrVal != nullptr)
            result.push_back(vtProp.bstrVal);
        if (hr == S_OK && vtProp.vt == VT_I4)
            result.push_back(std::to_wstring(vtProp.uintVal));
        
        VariantClear(&vtProp);
        
        pclsObj->Release();
    }
    pEnumerator->Release();
    return result;   // Program successfully completed.
}

std::unordered_multimap<std::wstring, std::wstring> WmiHelper::queryKeyValWMI(const char* query, LPCWSTR asKey, LPCWSTR asValue)
{
    HRESULT hres;
    std::unordered_multimap<std::wstring, std::wstring> result;

    // Step 6: --------------------------------------------------
    // Use the IWbemServices pointer to make requests of WMI ----

    // For example, get the name of the operating system
    IEnumWbemClassObject* pEnumerator = NULL;
    hres = _pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t(query),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres))
    {
        std::cout << "Query for operating system name failed."
            << " Error code = 0x"
            << std::hex << hres << std::endl;
        return result;               // Program has failed.
    }

    // Step 7: -------------------------------------------------
    // Get the data from the query in step 6 -------------------

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
            &pclsObj, &uReturn);

        if (0 == uReturn || hr != S_OK)
        {
            break;
        }

        VARIANT vtPropKey;
        VARIANT vtPropVal;

        VariantInit(&vtPropKey);
        VariantInit(&vtPropVal);
        // Get the value of the Name property
        HRESULT hrKey = pclsObj->Get(asKey, 0, &vtPropKey, 0, 0);
        HRESULT hrVal = pclsObj->Get(asValue, 0, &vtPropVal, 0, 0);

        std::wstring key, value;
        if (hrKey == S_OK && vtPropKey.vt == VT_BSTR && vtPropKey.bstrVal != nullptr)
            key = vtPropKey.bstrVal;
        if (hr == S_OK && vtPropKey.vt == VT_I4)
            key = std::to_wstring(vtPropKey.lVal);

        if (hrKey == S_OK && vtPropVal.vt == VT_BSTR && vtPropVal.bstrVal != nullptr)
            value = vtPropVal.bstrVal;
        if (hr == S_OK && vtPropVal.vt == VT_I4)
            value = std::to_wstring(vtPropVal.lVal);

        result.insert({ key, value });

        VariantClear(&vtPropKey);
        VariantClear(&vtPropVal);

        pclsObj->Release();
    }
    pEnumerator->Release();

    return result;   // Program successfully completed.
}