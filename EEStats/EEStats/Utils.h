#pragma once

#include <string>
#include <sstream>
#include <iostream>

static void replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

static void showMessage(std::string msg, std::string scope = "", bool error = false, bool show_time = true)
{
    std::stringstream ss;

    if (show_time) {
        struct tm newtime;
        __time64_t long_time;
        char buffer[80];
        errno_t err;

        time(&long_time);
        err = localtime_s(&newtime, &long_time);
        if (!err) {
            strftime(buffer, 80, "[%H:%M:%S | %d/%m/%Y] ", &newtime);
            ss << buffer;
        }
    }

    if (error)
        ss << "[ERROR] ";
    else
        ss << "[INFO] ";

    if (!scope.empty())
        ss << "(" << scope << ") ";
    ss << msg;

    if (error) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
        std::cerr << ss.str() << std::endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    }
    else {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
        std::cout << ss.str() << std::endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    }
}

static void showMessage(char* msg, std::string scope = "", bool error = false, bool show_time = true)
{
    showMessage(std::string(msg), scope, error, show_time); // Big brain moment
}