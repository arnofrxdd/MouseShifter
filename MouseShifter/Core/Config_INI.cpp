int currentGearLayout = 0; // index of active layout

void LoadGearLayoutsFromIni(const std::wstring& filename)
{
    gearLayouts.clear();
    gearLayoutNames.clear();

    std::wifstream file(filename);
    if (!file.is_open())
        return;

    std::wstring line;
    int layoutCount = 0;

    while (std::getline(file, line))
    {
        // Remove BOM if present
        if (!line.empty() && line[0] == 0xFEFF)
            line.erase(0, 1);

        // Trim whitespace
        line.erase(0, line.find_first_not_of(L" \t\r\n"));
        line.erase(line.find_last_not_of(L" \t\r\n") + 1);

        if (line.empty() || line[0] == L';' || line[0] == L'#')
            continue;

        if (line.find(L"count=") == 0)
        {
            layoutCount = std::stoi(line.substr(6));
        }
    }

    file.clear();
    file.seekg(0, std::ios::beg);

    for (int i = 0; i < layoutCount; ++i)
    {
        std::map<std::string, std::string> layout;
        std::wstring layoutName;
        std::wstring sectionHeader = L"[Layout" + std::to_wstring(i) + L"]";

        bool inSection = false;
        while (std::getline(file, line))
        {
            line.erase(0, line.find_first_not_of(L" \t\r\n"));
            line.erase(line.find_last_not_of(L" \t\r\n") + 1);

            if (line.empty() || line[0] == L';' || line[0] == L'#')
                continue;

            if (line == sectionHeader)
            {
                inSection = true;
                continue;
            }

            if (inSection)
            {
                if (line[0] == L'[')
                    break; // next section

                size_t eqPos = line.find(L'=');
                if (eqPos == std::wstring::npos)
                    continue;

                std::wstring key = line.substr(0, eqPos);
                std::wstring value = line.substr(eqPos + 1);

                key.erase(0, key.find_first_not_of(L" \t\r\n"));
                key.erase(key.find_last_not_of(L" \t\r\n") + 1);
                value.erase(0, value.find_first_not_of(L" \t\r\n"));
                value.erase(value.find_last_not_of(L" \t\r\n") + 1);

                if (key == L"name")
                    layoutName = value;
                else
                {
                    std::string keyStr(key.begin(), key.end());
                    std::string valueStr(value.begin(), value.end());
                    layout[keyStr] = valueStr;
                }
            }
        }

        if (!layout.empty())
        {
            gearLayouts.push_back(layout);
            gearLayoutNames.push_back(layoutName);
        }

        file.clear();
        file.seekg(0, std::ios::beg); // reset for next layout
    }
}
