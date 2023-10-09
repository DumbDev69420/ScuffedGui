#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <iostream>

class ParameterShit {
public:

    HANDLE TargetProcess; DWORD start; DWORD size; const char* sig; const char* mask;
    DWORD* returnaddrs;
    bool ReadMem = false;
    bool MinusBase = false;
    DWORD EditAddrs = 0x0;
    bool WasRead = false;



    void DefineObjects(HANDLE TargetProcess1, DWORD Start1, DWORD Size, const char* Signature, const char* Mask, DWORD* AddressToReturnTo, bool ReadMemory = false, bool ReturnOffsetMinusBaseAddress = false, DWORD EditAddressBy = 0x0) {
        TargetProcess = TargetProcess1;
        start = Start1;
        size = Size;
        sig = Signature;
        mask = Mask;
        returnaddrs = AddressToReturnTo;
        ReadMem = ReadMemory;
        MinusBase = ReturnOffsetMinusBaseAddress;
        EditAddrs = EditAddressBy;
    }
};


class ParameterShit64 {
public:


    HANDLE TargetProcess; uintptr_t start; uintptr_t size; const char* sig; const char* mask;

    uintptr_t* returnaddrs;
    bool ReadMem = false;
    bool MinusBase = false;
    uintptr_t EditAddrs = 0x0;
    bool WasRead = false;
    std::string ModuleName = "No Module Name :(";
    bool SkipFirstFind = false;
    bool GoToAddressFromStart = false;
    uintptr_t EditAddrdif = 0x0;

    void DefineObjects(HANDLE TargetProcess1, uintptr_t Start1, uintptr_t Size, const char* Signature, const char* Mask, uintptr_t* AddressToReturnTo, bool ReadMemory = false, bool ReturnOffsetMinusBaseAddress = false, uintptr_t EditAddressBy = 0x0) {
        TargetProcess = TargetProcess1;
        start = Start1;
        size = Size;
        sig = Signature;
        mask = Mask;
        returnaddrs = AddressToReturnTo;
        ReadMem = ReadMemory;
        MinusBase = ReturnOffsetMinusBaseAddress;
        EditAddrs = EditAddressBy;
    }
};

ParameterShit NewBugati;
ParameterShit64 NewBugatti1;
//External sig scanning functions where written by Zer0Mem0ry on github
namespace SigShit {

    void GetProcessMemoryInfoe(HANDLE Proc, DWORD* dwSize) {
        HANDLE process = GetCurrentProcess();
        PROCESS_MEMORY_COUNTERS_EX pmc;
        if (GetProcessMemoryInfo(process, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
            *dwSize = (DWORD)pmc.PeakWorkingSetSize;
        }
        else {
            // If the function fails, set the memory info to zero
            *dwSize = 0;
        }
        CloseHandle(process);
    }


    void GetProcessMemoryInfoeX64(HANDLE Proc, uintptr_t* dwSize) {
        HANDLE process = GetCurrentProcess();
        PROCESS_MEMORY_COUNTERS_EX pmc;
        if (GetProcessMemoryInfo(process, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
            *dwSize = (uintptr_t)pmc.PeakWorkingSetSize;
        }
        else {
            // If the function fails, set the memory info to zero
            *dwSize = 0;
        }
        CloseHandle(process);
    }

    // for comparing a region in memory, needed in finding a signature
    bool MemoryCompare(const BYTE* bData, const BYTE* bMask, const char* szMask) {
        for (; *szMask; ++szMask, ++bData, ++bMask) {
            if (*szMask == 'x' && *bData != *bMask) {
                return false;
            }
        }
        return (*szMask == NULL);
    }


    // for finding a signature/pattern in memory of another process
    DWORD FindSignature(HANDLE TargetProcess, DWORD start, DWORD size, const char* sig, const char* mask)
    {
        BYTE* data = new BYTE[size];
        SIZE_T bytesRead;

        ReadProcessMemory(TargetProcess, (LPVOID)start, data, size, &bytesRead);

        for (DWORD i = 0; i < size; i++)
        {
            if (MemoryCompare((const BYTE*)(data + i), (const BYTE*)sig, mask)) {
                return start + i;
            }
        }
        delete[] data;
        return NULL;
    }

    // for finding a signature/pattern in memory of another process
    uintptr_t FindSignature64(HANDLE TargetProcess, uintptr_t start, uintptr_t size, const char* sig, const char* mask, bool SkipFirstFind = false)
    {
        BYTE* data = new BYTE[size];
        SIZE_T bytesRead;

        ReadProcessMemory(TargetProcess, (LPVOID)start, data, size, &bytesRead);

        for (DWORD i = 0; i < size; i++)
        {
            if (MemoryCompare((const BYTE*)(data + i), (const BYTE*)sig, mask)) {
                if (SkipFirstFind == true) {
                    SkipFirstFind = false;
                    continue;
                }
                return start + i;
            }
        }
        delete[] data;
        return NULL;
    }

    void DebugPrint(ParameterShit& Amk);
    void DebugPrint64(ParameterShit64& Amk);

    bool WrapSig(ParameterShit& Doe) {
        bool Execute = true;

        if (Doe.start == 0 || Doe.TargetProcess == 0) {
            std::cout << "Invalid start address or Handle of Target proc!\n";
            Execute = false;
        }

        if (Doe.sig == "" || Doe.mask == "") {
            std::cout << "Invalid Sig or mask!\n";
            Execute = false;
        }

        if (Doe.size == 0) {
            std::cout << "Invalid size!\n";
        }

        CreateThread(nullptr, 0, LPTHREAD_START_ROUTINE(SigShit::DebugPrint), &Doe, 0, nullptr);
        return true;
    }

    bool WrapSig64(ParameterShit64& Doe) {
        bool Execute = true;

        if (Doe.start == 0 || Doe.TargetProcess == 0) {
            std::cout << "Invalid start address or Handle of Target proc!\n";
            Execute = false;
        }

        if (Doe.sig == "" || Doe.mask == "") {
            std::cout << "Invalid Sig or mask!\n";
            Execute = false;
        }

        if (Doe.size == 0) {
            std::cout << "Invalid size!\n";
        }

        CreateThread(nullptr, 0, LPTHREAD_START_ROUTINE(SigShit::DebugPrint64), &Doe, 0, nullptr);
        return true;
    }

    bool ReadAddrbackwards = false;

    void DebugPrint(ParameterShit& Amk)
    {
        DWORD Dot = 0x0;

        Dot = SigShit::FindSignature64(Amk.TargetProcess, Amk.start, Amk.size, Amk.sig, Amk.mask) + Amk.EditAddrs;
        if (Amk.ReadMem == false) {
            if (Amk.MinusBase == true) {
                *Amk.returnaddrs = (Dot - Amk.start);
                std::cout << "Base: " << std::hex << *Amk.returnaddrs << "\n";
                Amk.WasRead = true;
                return;
            }
            std::cout << "Base: " << std::hex << *Amk.returnaddrs << "\n";
            *Amk.returnaddrs = Dot;
            Amk.WasRead = true;
            return;
        }



        if (ReadAddrbackwards == true) {
            BYTE Dot2 = 0x0;
            DWORD Dot3 = 0x0;
            ReadProcessMemory(Amk.TargetProcess, (BYTE*)Dot, &Dot2, sizeof(Dot2), 0);
            for (size_t i = 0; i < 4; i++)
            {
                ReadProcessMemory(Amk.TargetProcess, (BYTE*)Dot - i + 3, &Dot2, sizeof(Dot2), 0);
                switch (i)
                {
                case 0:
                    Dot3 = Dot3 + (Dot2 * 0x1000000);
                    break;


                case 1:

                    Dot3 = Dot3 + (Dot2 * 0x10000);
                    break;


                case 2:

                    Dot3 = Dot3 + (Dot2 * 0x100);
                    break;



                case 3:

                    Dot3 = Dot3 + (Dot2 * 0x1);
                    break;
                }
            }
            Dot = Dot3;
        }
        else
        {
            ReadProcessMemory(Amk.TargetProcess, (BYTE*)Dot, &Dot, sizeof(Dot), 0);
        }

        if (Amk.MinusBase == true) {
            *Amk.returnaddrs = (Dot - Amk.start);
            Amk.WasRead = true;
            return;
        }

        *Amk.returnaddrs = Dot;
        Amk.WasRead = true;
    }

    void DebugPrint64(ParameterShit64& Amk)
    {
        uintptr_t Dot = 0x0;

        Dot = SigShit::FindSignature64(Amk.TargetProcess, Amk.start, Amk.size, Amk.sig, Amk.mask, Amk.SkipFirstFind) + Amk.EditAddrs;

        if (Amk.ModuleName == "EntityList_Pointer") {
            std::cout << "Address of EntityList:" << std::hex << Dot << "\n";
        }

        if (Dot - Amk.EditAddrs == 0x0) {
            std::cout << "Couldnt Find Address for {" + Amk.ModuleName + "}\n";
            *Amk.returnaddrs = 0x0;
            return;
        }

        if (Amk.GoToAddressFromStart == true) {
            volatile uintptr_t Sfd;
            Sfd = strnlen_s(Amk.mask, 2000) * 0x8;
            Dot = (Dot + Sfd);
        }

        if (Amk.ReadMem == false) {
            if (Amk.MinusBase == true) {
                *Amk.returnaddrs = (Dot - Amk.start);
                std::cout << "Base: " << std::hex << *Amk.returnaddrs << "\n";
                Amk.WasRead = true;
                return;
            }
            std::cout << "Base: " << std::hex << *Amk.returnaddrs << "\n";
            *Amk.returnaddrs = Dot;
            Amk.WasRead = true;
            return;
        }



        if (ReadAddrbackwards == true) {
            volatile uintptr_t Dot3 = 0x0;
            uintptr_t Dot6 = 0x0;

            for (size_t i = 0; i < 4; i++)
            {
                BYTE Dot2 = 0x0;
                ReadProcessMemory(Amk.TargetProcess, (BYTE*)Dot - i + 4, &Dot2, sizeof(Dot2), nullptr);

                switch (i)
                {
                case 0:
                    Dot6 = Dot2;
                    Dot3 = (0x10000000 * Dot6);
                    break;

                case 1:
                    Dot6 = Dot2;
                    Dot3 = Dot3 + (0x100000 * Dot6);
                    break;

                case 2:
                    Dot6 = Dot2;
                    Dot3 = Dot3 + (0x1000 * Dot6);
                    break;

                case 3:
                    Dot6 = Dot2;
                    Dot3 = Dot3 + (0x10 * Dot6);
                    break;
                }
            }

            Dot3 = static_cast<uintptr_t>(Dot3) >> 4;
            Dot = Dot - Amk.EditAddrs + Dot3;
            Dot = Dot + 0x7;


        }
        else
        {
            ReadProcessMemory(Amk.TargetProcess, (BYTE*)Dot, &Dot, sizeof(Dot), 0);
        }

        if (Amk.MinusBase == true) {
            if (Amk.EditAddrdif != 0x0) {
                *Amk.returnaddrs = (Dot - Amk.start + (Amk.EditAddrs - Amk.EditAddrdif));
            }
            else
            {
                *Amk.returnaddrs = (Dot - Amk.start);
            }

            Amk.WasRead = true;
            return;
        }

        *Amk.returnaddrs = Dot;
        Amk.WasRead = true;
    }
}
