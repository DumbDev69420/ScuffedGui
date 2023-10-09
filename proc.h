#pragma once
#include "icl.h"

DWORD GetProcId(const wchar_t* procName);
uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName);
HMODULE GetModuleHandleByArray(int Array);
HMODULE GetModuleHandleByName(std::wstring moduleName);
uintptr_t FindDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets);
uintptr_t FindDMAAddy32(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets);
DWORD GetNthChildProcessId(DWORD parentProcessId, int n);