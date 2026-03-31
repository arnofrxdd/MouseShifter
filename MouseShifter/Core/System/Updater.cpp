#include "Updater.h"
//update system
#include <string>
#include <algorithm>
#include <cstdio>
int CompareVersions(const std::string& a, const std::string& b)
{
    int majA = 0, minA = 0, majB = 0, minB = 0;
    sscanf_s(a.c_str(), "%d.%d", &majA, &minA);
    sscanf_s(b.c_str(), "%d.%d", &majB, &minB);


    if (majA != majB) return majA - majB;
    return minA - minB;
}

bool IsInternetAvailable()
{
    DWORD flags;
    return InternetGetConnectedState(&flags, 0) != 0;
}

std::string currentVersion = "6.2";
bool updateAvailable = false;
bool showUpdateModal = false;
std::string latestVersion = "";
RECT updateButtonRect;
RECT updateSubtleBtnRect; // Defined here for global access
bool checkingForUpdates = false;

#define DBGPRINT(fmt, ...) { char buf[512]; sprintf_s(buf, fmt, __VA_ARGS__); OutputDebugStringA(buf); }

std::string DownloadString(const std::string& url)
{
    HINTERNET hInternet = InternetOpenA("MouseShifter Updater", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return "";

    HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hUrl) { InternetCloseHandle(hInternet); return ""; }

    std::string result;
    char buffer[1024];
    DWORD bytesRead;

    while (InternetReadFile(hUrl, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0)
    {
        buffer[bytesRead] = '\0';
        result += buffer;
    }

    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);
    return result;
}

bool DownloadFile(const std::string& url, const std::wstring& localPath)
{
    HINTERNET hInternet = InternetOpenA("MouseShifter Updater", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return false;

    HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hUrl) { InternetCloseHandle(hInternet); return false; }

    HANDLE hFile = CreateFileW(localPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) { InternetCloseHandle(hUrl); InternetCloseHandle(hInternet); return false; }

    char buffer[8192];
    DWORD bytesRead, bytesWritten;
    bool success = true;

    while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0)
    {
        if (!WriteFile(hFile, buffer, bytesRead, &bytesWritten, NULL) || bytesWritten != bytesRead)
        {
            success = false;
            break;
        }
    }

    CloseHandle(hFile);
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);

    return success;
}
#include <filesystem>
namespace fs = std::filesystem;

bool ExtractZipNew(const std::wstring& zipPath, const std::wstring& extractPath)
{
    // Create update_temp folder
    std::wstring tempFolder = extractPath + L"\\update_temp";
    fs::create_directories(tempFolder);

    // Use PowerShell Expand-Archive
    std::wstring command = L"powershell -NoProfile -ExecutionPolicy Bypass -Command \"& {Try {Expand-Archive -Force -LiteralPath '"
        + zipPath + L"' -DestinationPath '" + tempFolder + L"' -ErrorAction Stop} Catch {exit 1}}\"";

    DBGPRINT("[Update] Extract command: %S\n", command.c_str());

    STARTUPINFOW si{ sizeof(si) };
    PROCESS_INFORMATION pi{};
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    wchar_t* cmd = _wcsdup(command.c_str());
    BOOL success = CreateProcessW(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    free(cmd);

    if (!success)
    {
        DBGPRINT("[Update] CreateProcessW failed, error: %lu\n", GetLastError());
        return false;
    }

    WaitForSingleObject(pi.hProcess, 60000);
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (exitCode != 0)
    {
        DBGPRINT("[Update] Extract process exited with code: %lu\n", exitCode);
        return false;
    }

    DBGPRINT("[Update] Extraction successful to: %S\n", tempFolder.c_str());
    return true;
}


std::string latestTag = ""; // stores GitHub tag (v2.0-update)
#include <json/json.h>
#include <sstream> // for std::istringstream

void CheckForUpdates()
{
    if (!IsInternetAvailable())
    {
        DBGPRINT("[Update] No internet connection, skipping update check.\n");
        return;
    }

    checkingForUpdates = true;

    std::string apiUrl = "https://api.github.com/repos/arnofrxdd/MouseShifter/releases/latest";
    std::string jsonResponse = DownloadString(apiUrl);

    if (!jsonResponse.empty())
    {
        Json::Value root;
        Json::CharReaderBuilder builder;
        builder["collectComments"] = false; // optional
        std::string errs;
        std::istringstream s(jsonResponse);

        if (Json::parseFromStream(builder, s, &root, &errs))
        {
            if (root.isMember("tag_name"))
            {
                latestTag = root["tag_name"].asString(); // e.g. "v2.0-update"

                // Convert "v2.0-update" -> "2.0" for numeric comparison
                size_t pos = latestTag.find_first_of("0123456789");
                size_t endPos = latestTag.find_first_of("-", pos);
                std::string numericVer = latestTag.substr(pos, endPos - pos);
                latestVersion = numericVer;

                if (CompareVersions(latestVersion, currentVersion) > 0)
                {
                    updateAvailable = true;
                    DBGPRINT("[Update] New version available: %s (current: %s)\n",
                        latestVersion.c_str(), currentVersion.c_str());
                }
                else
                {
                    updateAvailable = false;
                    DBGPRINT("[Update] No new version (latest: %s, current: %s)\n",
                        latestVersion.c_str(), currentVersion.c_str());
                }
            }
        }
        else
        {
            DBGPRINT("[Update] Failed to parse GitHub API JSON: %s\n", errs.c_str());
        }
    }

    checkingForUpdates = false;
}



void PerformUpdate()
{
    if (latestTag.empty())
    {
        DBGPRINT("[Update] No latest tag info available.\n");
        return;
    }

    // Generate dynamic download URL from tag
    std::string zipUrl = "https://github.com/arnofrxdd/MouseShifter/releases/download/" + latestTag + "/MouseShifter_Release.zip";
    std::wstring tempZip = L"MouseShifter_update.zip";
    std::wstring exePath, updaterPath;

    wchar_t path[MAX_PATH];
    if (GetModuleFileNameW(NULL, path, MAX_PATH) == 0)
    {
        DBGPRINT("[Update] GetModuleFileNameW failed, error: %lu\n", GetLastError());
        return;
    }

    exePath = path;
    std::wstring exeDir = exePath.substr(0, exePath.find_last_of(L"\\/"));
    std::wstring tempZipPath = exeDir + L"\\" + tempZip;
    std::wstring tempFolderW = exeDir + L"\\update_temp";
    std::wstring logPathW = exeDir + L"\\update_log.txt";

    DBGPRINT("[Update] Downloading zip from: %s\n", zipUrl.c_str());

    if (!DownloadFile(zipUrl, tempZipPath))
    {
        DBGPRINT("[Update] DownloadFile failed, error: %lu\n", GetLastError());
        MessageBoxA(NULL, "Failed to download the update. Check your internet connection.", "Update Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Modern "Hand-Off" Update Strategy:
    // This is the "God Script": It waits for MouseShifter to die, extracts, cleans, and restarts.
    std::wstring psCommand = 
        L"-NoProfile -ExecutionPolicy Bypass -Command \""
        L"Start-Sleep -s 1; "
        L"$log = '" + logPathW + L"'; "
        L"\"--- Starting Update --- \" | Out-File $log; "
        L"$proc = Get-Process -Name 'MouseShifter' -ErrorAction SilentlyContinue; "
        L"if ($proc) { \"Closing process...\" | Out-File $log -Append; $proc | Stop-Process -Force; Start-Sleep -s 1; } "
        L"$zip = '" + tempZipPath + L"'; "
        L"$dest = '" + exeDir + L"'; "
        L"$temp = '" + tempFolderW + L"'; "
        L"try { "
        L"  \"Extracting $zip to $temp...\" | Out-File $log -Append; "
        L"  Expand-Archive -Path $zip -DestinationPath $temp -Force; "
        L"  \"Moving files to $dest...\" | Out-File $log -Append; "
        L"  Get-ChildItem -Path $temp -Recurse | ForEach-Object { "
        L"    $target = Join-Path $dest $_.Fullname.Substring($temp.Length + 1); "
        L"    if ($_.PsIsContainer) { if (!(Test-Path $target)) { New-Item $target -ItemType Directory | Out-Null } } "
        L"    else { "
        L"       if ($_.Name -notmatch 'config.ini|gearlayouts.ini|profiles/') { "
        L"          \"Replacing: $($_.Name)\" | Out-File $log -Append; "
        L"          Copy-Item $_.FullName $target -Force; "
        L"       } "
        L"    } "
        L"  }; "
        L"  \"Update Complete!\" | Out-File $log -Append; "
        L"} catch { \"ERROR: $($_.Exception.Message)\" | Out-File $log -Append; } finally { "
        L"  Remove-Item $temp -Recurse -Force -ErrorAction SilentlyContinue; "
        L"  Remove-Item $zip -Force -ErrorAction SilentlyContinue; "
        L"  Start-Process -FilePath '" + exePath + L"'; "
        L"} \"";

    DBGPRINT("[Update] Launching Modern PS Dispatcher...\n");

    // Launch PowerShell directly without a batch file
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE; // Run it silently

    wchar_t psPath[MAX_PATH];
    ExpandEnvironmentStringsW(L"%SystemRoot%\\System32\\WindowsPowerShell\\v1.0\\powershell.exe", psPath, MAX_PATH);

    std::wstring fullCmd = L"\"" + std::wstring(psPath) + L"\" " + psCommand;
    wchar_t* cmdBuffer = _wcsdup(fullCmd.c_str());

    if (CreateProcessW(NULL, cmdBuffer, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        free(cmdBuffer);
        
        DBGPRINT("[Update] Hand-off successful. Exiting app.\n");
        // Exit fast to let PS take over
        _exit(0);
    }
    else
    {
        free(cmdBuffer);
        DBGPRINT("[Update] CreateProcessW failed, error: %lu\n", GetLastError());
        MessageBoxW(NULL, L"The update dispatcher failed to start. Try running as Admin.", L"Update Hand-off Failed", MB_OK | MB_ICONERROR);
    }
}



