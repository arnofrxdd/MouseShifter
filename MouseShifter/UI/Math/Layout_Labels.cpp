

RECT gearLayoutButtonRect;
bool gearLayoutDropdownOpen = false;

// Define the gear label map (can be updated later to override labels)
std::map<std::string, std::string> gearLabelOverride = {
    {"1", "1"}, {"2", "2"}, {"3", "3"}, {"4", "4"}, {"5", "5"}, {"6", "6"}, {"7", "7"}, {"8", "8"}, {"9", "9"}, {"10", "10"}, {"11", "11"}, {"12", "12"}, {"13", "13"}, {"14", "14"}, {"15", "15"}, {"16", "16"}, {"R", "R"} };

// Add at global scope or before showSettingsPanel block
std::vector<std::map<std::string, std::string>> gearLayouts = {
    {// default
     {"1", "1"},
     {"2", "2"},
     {"3", "3"},
     {"4", "4"},
     {"5", "5"},
     {"6", "6"},
     {"7", "7"},
     {"8", "8"},
     {"9", "9"},
     {"10", "10"},
     {"11", "11"},
     {"12", "12"},
     {"13", "13"},
     {"14", "14"},
     {"15", "15"},
     {"16", "16"},
     {"R", "R"}},
    {// Alphabet
     {"1", "A"},
     {"2", "B"},
     {"3", "C"},
     {"4", "D"},
     {"5", "E"},
     {"6", "F"},
     {"7", "G"},
     {"8", "H"},
     {"9", "I"},
     {"10", "J"},
     {"11", "K"},
     {"12", "L"},
     {"13", "M"},
     {"14", "N"},
     {"15", "O"},
     {"16", "P"},
     {"R", "R"}},
    {// Roman Numerals
     {"1", "I"},
     {"2", "II"},
     {"3", "III"},
     {"4", "IV"},
     {"5", "V"},
     {"6", "VI"},
     {"7", "VII"},
     {"8", "VIII"},
     {"9", "IX"},
     {"10", "X"},
     {"11", "XI"},
     {"12", "XII"},
     {"13", "XIII"},
     {"14", "XIV"},
     {"15", "XV"},
     {"16", "XVI"},
     {"R", "R"}} };

std::vector<std::wstring> gearLayoutNames = {
    L"Default",       // gearLayouts[0]
    L"Alphabet",      // gearLayouts[1]
    L"Roman Numerals" // gearLayouts[2]
};
