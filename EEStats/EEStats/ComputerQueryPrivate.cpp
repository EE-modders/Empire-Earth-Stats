#include "pch.h"

#include "ComputerQuery.h"
#include "Utils.h"

#include "sha512.h"

std::string ComputerQuery::getBiosSerial()
{
     return "";
}

std::string ComputerQuery::getWindowsDiskSerial()
{
     return "";
}

std::string ComputerQuery::getComputerSerial()
{
     return "";
}

std::string ComputerQuery::getUID()
{
     return "";
}

bool ComputerQuery::runInVirtualPC() // Who use that ?
{
     return false;
}


bool ComputerQuery::runInVMWare()
{
     return false;
}

bool ComputerQuery::runInVirtualBox()
{
     return false;
}

bool ComputerQuery::runInOtherVM()
{
     return false;
}

bool ComputerQuery::runInVM()
{
    if (runInOtherVM() || runInVMWare() || runInVirtualBox() || runInVirtualPC())
        return true;
    return false;
}