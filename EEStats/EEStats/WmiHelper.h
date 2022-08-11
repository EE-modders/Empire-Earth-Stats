#pragma once
#include <wtypes.h>
#include <oleauto.h>

std::wstring queryWMI(const char* query, LPCWSTR value);