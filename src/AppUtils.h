// CommitMonitor - simple checker for new commits in svn repositories

// Copyright (C) 2007-2010 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#pragma once

#include <string>
#include <vector>
#include "svn_time.h"

using namespace std;

class CAppUtils
{
public:
    CAppUtils(void);
    ~CAppUtils(void);

    static wstring                  GetDataDir();
    static wstring                  GetAppDataDir();
    static wstring                  GetAppDirectory(HMODULE hMod = NULL);
    static wstring                  GetAppName(HMODULE hMod = NULL);
    static wstring                  ConvertDate(apr_time_t time);
    static void                     SearchReplace(wstring& str, const wstring& toreplace, const wstring& replacewith);
    static vector<wstring>          tokenize_str(const wstring& str, const wstring& delims);
    static bool                     LaunchApplication(const wstring& sCommandLine, bool bWaitForStartup = false);
    static wstring                  GetTempFilePath();
    static wstring                  ConvertName(const wstring& name);
    static string                   PathEscape(const string& path);
    static bool                     IsWow64();
    static wstring                  GetTSVNPath();
    static wstring                  GetVersionStringFromExe(LPCTSTR path);
    static bool                     ExtractBinResource(const wstring& strCustomResName, int nResourceId, const wstring& strOutputPath);
    static bool                     WriteAsciiStringToClipboard(const wstring& sClipdata, HWND hOwningWnd);
    static bool                     IsFullscreenWindowActive();
};