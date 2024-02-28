
#ifndef DSINFER_STRING_TYPE_H
#define DSINFER_STRING_TYPE_H

#include <string>

#ifdef _WIN32
#define TSTR(x) L##x
using TString = std::wstring;
using TChar = wchar_t;
#else
#define TSTR(x) x
using TString = std::string;
using TChar = char;
#endif

#endif // DSINFER_STRING_TYPE_H
