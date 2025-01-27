/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2019 by Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

  This file is a part of DOOM Retro.

  DOOM Retro is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM Retro is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM Retro. If not, see <https://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries, and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#if defined(_WIN32)
#include <Windows.h>

void I_ShutdownWindows32(void);
#else
#include <unistd.h>
#endif

#include "c_console.h"
#include "d_main.h"
#include "i_gamepad.h"
#include "i_timer.h"
#include "m_config.h"
#include "m_misc.h"
#include "s_sound.h"
#include "version.h"

extern dboolean returntowidescreen;

#if defined(_WIN32)
extern char     *previouswad;

typedef long (__stdcall *PRTLGETVERSION)(PRTL_OSVERSIONINFOEXW);
typedef BOOL (WINAPI *PGETPRODUCTINFO)(DWORD, DWORD, DWORD, DWORD, PDWORD);
typedef BOOL (WINAPI *PISWOW64PROCESS)(HANDLE, PBOOL);

#define PRODUCT_CORE    0x00000065

void I_PrintWindowsVersion(void)
{
    PRTLGETVERSION  pRtlGetVersion = (PRTLGETVERSION)GetProcAddress(GetModuleHandle("ntdll.dll"), "RtlGetVersion");
    PGETPRODUCTINFO pGetProductInfo = (PGETPRODUCTINFO)GetProcAddress(GetModuleHandle("kernel32.dll"), "GetProductInfo");
    PISWOW64PROCESS pIsWow64Process = (PISWOW64PROCESS)GetProcAddress(GetModuleHandle("kernel32.dll"), "IsWow64Process");

    if (pRtlGetVersion && pGetProductInfo)
    {
        char                bits[10] = "";
        char                *typename = "";
        OSVERSIONINFOEXW    info;
        DWORD               type;

        if (pIsWow64Process)
        {
            BOOL    Wow64Process = FALSE;

            pIsWow64Process(GetCurrentProcess(), &Wow64Process);
            strcpy(bits, (Wow64Process || sizeof(intptr_t) == 8 ? "64-bit" : "32-bit"));
        }

        ZeroMemory(&info, sizeof(OSVERSIONINFOEXW));
        info.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);

        pRtlGetVersion((PRTL_OSVERSIONINFOEXW)&info);

        pGetProductInfo(info.dwMajorVersion, info.dwMinorVersion, 0, 0, &type);

        switch (type)
        {
            case PRODUCT_ULTIMATE:
                typename = "Ultimate";
                break;

            case PRODUCT_PROFESSIONAL:
                typename = "Professional";
                break;

            case PRODUCT_HOME_PREMIUM:
                typename = "Home Premium";
                break;

            case PRODUCT_HOME_BASIC:
                typename = "Home Basic";
                break;

            case PRODUCT_ENTERPRISE:
                typename = "Enterprise";
                break;

            case PRODUCT_BUSINESS:
                typename = "Business";
                break;

            case PRODUCT_STARTER:
                typename = "Starter";
                break;

            case PRODUCT_CLUSTER_SERVER:
                typename = "Cluster Server";
                break;

            case PRODUCT_DATACENTER_SERVER:
            case PRODUCT_DATACENTER_SERVER_CORE:
                typename = "Datacenter Edition";
                break;

            case PRODUCT_ENTERPRISE_SERVER:
            case PRODUCT_ENTERPRISE_SERVER_CORE:
            case PRODUCT_ENTERPRISE_SERVER_IA64:
                typename = "Enterprise";
                break;

            case PRODUCT_SMALLBUSINESS_SERVER:
                typename = "Small Business Server";
                break;

            case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
                typename = "Small Business Server Premium";
                break;

            case PRODUCT_STANDARD_SERVER:
            case PRODUCT_STANDARD_SERVER_CORE:
                typename = "Standard";
                break;

            case PRODUCT_WEB_SERVER:
                typename = "Web Server";
                break;

            case PRODUCT_CORE:
                typename = "Home";
        }

        if (info.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
            char    *infoname = "NT";

            if (info.dwMajorVersion == 5)
            {
                if (info.dwMinorVersion == 0)
                    infoname = "2000";
                else if (info.dwMinorVersion == 1)
                    infoname = "XP";
                else if (info.dwMinorVersion == 2)
                    infoname = "Server 2003";
            }
            else if (info.dwMajorVersion == 6)
            {
                if (info.dwMinorVersion == 0)
                    infoname = (info.wProductType == VER_NT_WORKSTATION ? "Vista" : "Server 2008");
                else if (info.dwMinorVersion == 1)
                    infoname = (info.wProductType == VER_NT_WORKSTATION ? "7" : "Server 2008 R2");
                else if (info.dwMinorVersion == 2)
                    infoname = (info.wProductType == VER_NT_WORKSTATION ? "8" : "Server 2012");
                else if (info.dwMinorVersion == 3)
                    infoname = "8.1";
            }
            else if (info.dwMajorVersion == 10)
                infoname = (info.wProductType == VER_NT_WORKSTATION ? "10" : "Server 2016");

            C_Output("Running on %s <i><b>Microsoft Windows %s%s%s%s%ws%s (Build %s)</b></i>.",
                bits, infoname, (*typename ? " " : ""), typename, (wcslen(info.szCSDVersion) ? " (" : ""),
                (wcslen(info.szCSDVersion) ? info.szCSDVersion : L""), (wcslen(info.szCSDVersion) ? ")" : ""),
                commify(info.dwBuildNumber));
        }
    }
}
#endif

void I_PrintSystemInfo(void)
{
    int     cores = SDL_GetCPUCount();
    char    *ram = commify(SDL_GetSystemRAM() / 1000);

    C_Output("There %s %i logical core%s and %sGB of system RAM.", (cores > 1 ? "are" : "is"), cores, (cores > 1 ? "s" : ""), ram);
    free(ram);
}

//
// I_Quit
//
void I_Quit(dboolean shutdown)
{
    if (shutdown)
    {
        D_FadeScreen();

        S_Shutdown();

        if (returntowidescreen)
            vid_widescreen = true;

        M_SaveCVARs();

        I_ShutdownGraphics();
        I_ShutdownKeyboard();
        I_ShutdownGamepad();
        I_ShutdownTimer();
    }

#if defined(_WIN32)
    I_ShutdownWindows32();
#endif

    exit(0);
}

//
// I_Error
//
void I_Error(const char *error, ...)
{
    va_list         argptr;
    char            msgbuf[512];
    static dboolean already_quitting;

    if (already_quitting)
        exit(-1);
    else
        already_quitting = true;

    // Shutdown. Here might be other errors.
    S_Shutdown();

    if (returntowidescreen)
        vid_widescreen = true;

#if defined(_WIN32)
    wad = M_StringDuplicate(previouswad);
#endif

    M_SaveCVARs();

    I_ShutdownGraphics();
    I_ShutdownKeyboard();
    I_ShutdownGamepad();
    I_ShutdownTimer();

#if defined(_WIN32)
    I_ShutdownWindows32();
#endif

    va_start(argptr, error);
    vfprintf(stderr, error, argptr);
    fprintf(stderr, "\n\n");
    va_end(argptr);
    fflush(stderr);

    va_start(argptr, error);
    memset(msgbuf, 0, sizeof(msgbuf));
    M_vsnprintf(msgbuf, sizeof(msgbuf) - 1, error, argptr);
    va_end(argptr);

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, PACKAGE_NAME, msgbuf, NULL);

    exit(-1);
}

//
// I_Realloc
//
void *I_Realloc(void *ptr, size_t size)
{
    void    *newp = realloc(ptr, size);

    if (!newp && size)
        I_Error("I_Realloc: Failure trying to reallocate %i bytes", size);
    else
        ptr = newp;

    return ptr;
}
