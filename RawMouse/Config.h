#pragma once
#include <string>
#include <windows.h>

void SaveConfig();
void LoadConfig();
void InitializeProfiles();
void RefreshProfilesList();
void CreateNewProfile(HWND hwnd);
void SwitchProfile(int index, HWND hwnd);
