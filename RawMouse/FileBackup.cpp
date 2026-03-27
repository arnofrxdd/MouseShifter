void backupAndEdit(const fs::path& filePath)
{
    fs::path backupPath = filePath;
    backupPath += L"_backup_controls.sii";

    // --- Only create backup if it doesn't exist ---
    if (!fs::exists(backupPath))
    {
        std::wifstream inFile(filePath);
        if (!inFile.is_open())
            return; // Skip if file can't be opened

        std::wofstream backup(backupPath);
        if (!backup.is_open())
            return; // Skip if backup file can't be created

        std::wstring line;
        while (std::getline(inFile, line))
        {
            backup << line << L"\n";
        }
        inFile.close();
        backup.close();
    }

    // --- Read original file into memory for editing ---
    std::wifstream inFile(filePath);
    if (!inFile.is_open())
        return;

    std::vector<std::wstring> lines;
    std::wstring line;
    while (std::getline(inFile, line))
    {
        lines.push_back(line);
    }
    inFile.close();

    // --- Edit target lines ---
    for (size_t i = 0; i < lines.size(); i++)
    {
        for (size_t j = 0; j < targetLines.size(); j++)
        {
            if (lines[i].find(targetLines[j]) != std::wstring::npos)
            {
                size_t quotePos = lines[i].find(L'"');
                if (quotePos != std::wstring::npos)
                {
                    lines[i] = lines[i].substr(0, quotePos + 1) + replacementLines[j] + L'"';
                }
            }
        }
    }

    // --- Write edited file ---
    std::wofstream outFile(filePath);
    if (!outFile.is_open())
        return;
    for (auto& l : lines)
        outFile << l << L"\n";
}

// Revert function
void revertFromBackup(const fs::path& filePath)
{
    std::wifstream backup(filePath.wstring() + L"_backup_controls.sii");
    if (!backup.is_open())
        return; // Skip if backup doesn't exist

    std::wifstream inFile(filePath);
    if (!inFile.is_open())
        return; // Skip if original file doesn't exist

    std::vector<std::wstring> backupLines, lines;
    std::wstring line;

    while (std::getline(backup, line))
        backupLines.push_back(line);
    while (std::getline(inFile, line))
        lines.push_back(line);
    backup.close();
    inFile.close();

    for (size_t i = 0; i < lines.size(); i++)
    {
        for (size_t j = 0; j < targetLines.size(); j++)
        {
            if (lines[i].find(targetLines[j]) != std::wstring::npos)
            {
                size_t quotePos = lines[i].find(L'"');
                if (quotePos != std::wstring::npos)
                {
                    size_t endQuotePos = backupLines[i].rfind(L'"');
                    if (endQuotePos != std::wstring::npos && endQuotePos > quotePos)
                    {
                        lines[i] = lines[i].substr(0, quotePos + 1) +
                            backupLines[i].substr(quotePos + 1, endQuotePos - quotePos - 1) +
                            L'"';
                    }
                }
            }
        }
    }

    std::wofstream outFile(filePath);
    if (!outFile.is_open())
        return; // Skip if can't write
    for (auto& l : lines)
        outFile << l << L"\n";
}

// Process all files for both ATS and ETS2
void processAllFiles(bool edit)
{
    std::vector<fs::path> folders = { L"profiles", L"steam_profiles" };

    for (auto& gamePath : gamePaths)
    {
        if (!fs::exists(gamePath))
            continue; // skip if game folder doesn't exist

        for (auto& folder : folders)
        {
            fs::path folderPath = gamePath / folder;
            if (!fs::exists(folderPath))
                continue; // skip if subfolder doesn't exist

            for (auto& p : fs::recursive_directory_iterator(folderPath))
            {
                if (p.path().filename() == L"controls.sii")
                {
                    if (edit)
                        backupAndEdit(p.path());
                    else
                        revertFromBackup(p.path());
                }
            }
        }
    }
}
