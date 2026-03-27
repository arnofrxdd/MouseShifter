
void RefreshProfilesList();

void InitializeProfiles() {
    std::string exeFolder = GetExeFolder();
    std::string profilesDir = exeFolder + "\\profiles";
    std::string oldConfig = exeFolder + "\\config.ini";
    std::string defaultProfile = profilesDir + "\\Default Profile.ini";

    // Create profiles directory if it doesn't exist
    CreateDirectoryA(profilesDir.c_str(), NULL);

    // Check if old config exists and move it
    if (GetFileAttributesA(oldConfig.c_str()) != INVALID_FILE_ATTRIBUTES) {
        // Move old config to profiles folder
        if (MoveFileA(oldConfig.c_str(), defaultProfile.c_str())) {
            OutputDebugStringA("[PROFILES] Migrated config.ini to profiles/Default Profile.ini\n");
        }
    }

    // Scan for existing profiles
    RefreshProfilesList();

    // Update configFile to point to current profile
    if (!profileNames.empty()) {
        configFile = profilesDir + "\\" + profileNames[currentProfileIndex];
    }
}

void RefreshProfilesList() {
    profileNames.clear();
    std::string profilesDir = GetExeFolder() + "\\profiles";

    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFileA((profilesDir + "\\*.ini").c_str(), &findFileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                profileNames.push_back(findFileData.cFileName);
            }
        } while (FindNextFileA(hFind, &findFileData));
        FindClose(hFind);
    }

    // If no profiles found, create a default one
    if (profileNames.empty()) {
        std::string defaultProfile = profilesDir + "\\Default Profile.ini";
        // Create empty file
        HANDLE hFile = CreateFileA(defaultProfile.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            CloseHandle(hFile);
            profileNames.push_back("Default Profile.ini");
        }
    }
}

void CreateNewProfile(HWND hwnd) {
    std::string profilesDir = GetExeFolder() + "\\profiles";
    std::string newProfilePath = profilesDir + "\\" + newProfileName + ".ini";

    // Create new profile file
    HANDLE hFile = CreateFileA(newProfilePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);

        // Refresh list and switch to new profile
        RefreshProfilesList();

        // Find the new profile index
        for (size_t i = 0; i < profileNames.size(); ++i) {
            if (profileNames[i] == newProfileName + ".ini") {
                currentProfileIndex = i;
                configFile = profilesDir + "\\" + profileNames[currentProfileIndex];

                // Load default settings instead of saving current ones
                LoadConfig(); // This will load default values since the file is empty

                break;
            }
        }

        creatingNewProfile = false;
        newProfileName = "New Profile";

        // Recompute layout after profile switch
        ComputeLayout(hwnd);
        ComputeIntersections();
    }
}

void SwitchProfile(int index, HWND hwnd) {
    if (index >= 0 && index < (int)profileNames.size()) {
        // Save current profile before switching
        SaveConfig();

        currentProfileIndex = index;
        std::string profilesDir = GetExeFolder() + "\\profiles";
        configFile = profilesDir + "\\" + profileNames[currentProfileIndex];
        LoadConfig();

        // Recompute layout
        ComputeLayout(hwnd);
        ComputeIntersections();
    }
}
