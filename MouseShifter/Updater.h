#pragma once
#include <string>
#include <windows.h>

extern std::string currentVersion;
extern bool updateAvailable;
extern std::string latestVersion;
extern RECT updateButtonRect;
extern bool checkingForUpdates;

int CompareVersions(const std::string& a, const std::string& b);
bool IsInternetAvailable();
std::string DownloadString(const std::string& url);
bool DownloadFile(const std::string& url, const std::wstring& localPath);
bool ExtractZipNew(const std::wstring& zipPath, const std::wstring& extractPath);
void CheckForUpdates();
void PerformUpdate();

