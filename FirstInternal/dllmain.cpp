#include <windows.h>
#include <iostream>
#include <TlHelp32.h>
#include <math.h>
#include "mem.h"


#define PI 3.14159265359


DWORD WINAPI HackThread(HMODULE hModule)
{
    //Create Console
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    std::cout << "[*] Injected :)\n";

    
    //Get Module Base

    uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"OMFGame-Win64-Shipping.exe");
    
    uintptr_t posAddy = mem::FindDMAAddy(moduleBase + 0x03FE72B0, {0x118, 0xA0, 0x150, 0x1D0});
    
    uintptr_t jmptoaddy;
    bool noclip = 0;
    float* x = (float*)(posAddy);
    float* y = (float*)(posAddy + 0x4);
    float* z = (float*)(posAddy + 0x8);
    float* pitch = (float*)(posAddy - 0xA4);
    
    //create codecave
    BYTE* gateway = (BYTE*)VirtualAlloc(0, 0x50, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    //setup codecave bytecode
    uintptr_t jmpbackaddy = moduleBase + 0x1F92A53 + 0xE;
    memcpy(gateway, "\x66\x9c\x53\x48\xBB", 5);
    memcpy(gateway + 5, (void*)&posAddy, 8);
    memcpy(gateway + 5 + 8, "\x48\x81\xEB\xD0\x01\x00\x00\x48\x39\xDF\x0F\x84\x08\x00\x00\x00\x44\x0F\x11\xA7\xD0\x01\x00\x00\x5B\x66\x9D\xB2\x01\x44\x0F\x11\xBF\xE0\x01\x00\x00\xFF\x25\x00\x00\x00\x00", 43); //make sure to pop rbx later! \x5B
    memcpy(gateway + 5 + 8 + 43, (void*)&jmpbackaddy, 8); //xFF\x25 jmp code
  
    //prints out the addy to the gateway (for debugging reasons)
    std::cout << std::hex << (uintptr_t)gateway << std::endl;

    //main loop
    while (!GetAsyncKeyState(VK_END))
    {
        

        if (GetAsyncKeyState(0x56) & 1) //V
        {
            

            noclip = !noclip;
            if (noclip)
            {
                posAddy = mem::FindDMAAddy(moduleBase + 0x03FE72B0, { 0x118, 0xA0, 0x150, 0x1D0 });
                x = (float*)(posAddy);
                y = (float*)(posAddy + 0x4);
                z = (float*)(posAddy + 0x8);
                pitch = (float*)(posAddy - 0xA4);
                memcpy(gateway + 5, (void*)&posAddy, 8);

                jmptoaddy = (moduleBase + 0x1F92A53) - (uintptr_t)gateway;
                //enable detour
                DWORD oldprotect;
                VirtualProtect((BYTE*)(moduleBase + 0x1F92A53), 18, PAGE_EXECUTE_READWRITE, &oldprotect);

                BYTE jmpcode[18];
                memset(jmpcode, 0x90, 18);
                memcpy(jmpcode, "\xFF\x25\x00\x00\x00\x00", 6);
                memcpy(jmpcode + 6, &gateway, 8);
                //write byte array into memory
                memcpy((BYTE*)(moduleBase + 0x1F92A53), jmpcode, 18);
              
                VirtualProtect((BYTE*)(moduleBase + 0x1F92A53), 18, oldprotect, &oldprotect);
               
            }
            else
            {
                //disable detour
                mem::Patch((BYTE*)moduleBase + 0x1F92A53, (BYTE*)"\x44\x0F\x11\xA7\xD0\x01\x00\x00\xB2\x01\x44\x0F\x11\xBF\xE0\x01\x00\x00", 18);
                
            }
        }
        if (noclip)
        {
            int speed = 10;
            if (GetAsyncKeyState(0x57)) //w
            {
                
                *x = *x + cos(*pitch * (PI / 180)) * 10;
                *y = *y + sin(*pitch * (PI / 180)) * 10;
            }
            if (GetAsyncKeyState(0x53)) //s
            {
                *x = *x - cos(*pitch * (PI / 180)) * 10;
                *y = *y - sin(*pitch * (PI / 180)) * 10;
            }
            if (GetAsyncKeyState(0x51)) //q
            {
                *z = *z - speed;
            }
            if (GetAsyncKeyState(0x45)) //e
            {
                *z = *z + speed;
            }
            
        }
        
        Sleep(5);
    }
    

    //Cleanup & Eject
    fclose(f);
    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
    return 0;

}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, nullptr));
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

