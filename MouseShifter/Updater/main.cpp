#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

// Log function to track what's happening
void Log(const std::string& msg)
{
    FILE* f = NULL;
    fopen_s(&f, "update_log.txt", "a");
    if (f)
    {
        fprintf(f, "%s\n", msg.c_str());
        fclose(f);
    }
}

bool IsProcessRunning(DWORD pid)
{
    HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid);
    if (process)
    {
        DWORD wait = WaitForSingleObject(process, 100);
        CloseHandle(process);
        return wait == WAIT_TIMEOUT;
    }
    return false;
}

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        return 1;
    }

    DWORD parentPid = std::stoul(argv[1]);
    std::string zipPath = argv[2];
    std::string destDir = argv[3];

    Log("--- Starting Updater Project ---");
    Log("Parent PID: " + std::to_string(parentPid));
    Log("Zip Path: " + zipPath);
    Log("Dest Dir: " + destDir);

    // 1. Wait for parent to exit
    int retries = 50; // 5 seconds max
    while (IsProcessRunning(parentPid) && retries-- > 0)
    {
        Sleep(100);
    }
    
    if (retries <= 0) {
       Log("Failed to wait for parent to exit, trying to force kill...");
       HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, parentPid);
       if (h) { TerminateProcess(h, 0); CloseHandle(h); }
       Sleep(1000);
    }

    // 2. Extract using tar.exe (built-in Windows tool)
    std::string tempDir = "update_temp";
    fs::create_directories(tempDir);
    
    std::string tarCmd = "tar -xf \"" + zipPath + "\" -C \"" + tempDir + "\"";
    Log("Extracting: " + tarCmd);
    
    int tarResult = system(tarCmd.c_str());
    if (tarResult != 0)
    {
        Log("tar extraction failed with code: " + std::to_string(tarResult));
        MessageBoxA(NULL, "Failed to extract update package. Check update_log.txt", "MouseShifter Updater", MB_OK | MB_ICONERROR);
        return 1;
    }

    // 3. Copy files (excluding configs)
    try {
        for (const auto& entry : fs::recursive_directory_iterator(tempDir))
        {
            if (entry.is_directory()) continue;

            fs::path relPath = fs::relative(entry.path(), tempDir);
            std::string fileName = relPath.filename().string();
            std::string fullRelPath = relPath.string();

            // Exclude these files/paths from being overwritten
            if (fullRelPath.find("config.ini") != std::string::npos ||
                fullRelPath.find("gearlayouts.ini") != std::string::npos ||
                fullRelPath.find("profiles/") != std::string::npos ||
                fullRelPath.find("profiles\\") != std::string::npos)
            {
                Log("Skipping: " + fullRelPath);
                continue;
            }

            fs::path targetPath = fs::path(destDir) / relPath;
            fs::create_directories(targetPath.parent_path());

            Log("Replacing: " + targetPath.string());
            
            // Try to move/copy
            std::error_code ec;
            fs::copy_file(entry.path(), targetPath, fs::copy_options::overwrite_existing, ec);
            if (ec)
            {
                Log("Error replacing " + targetPath.string() + ": " + ec.message());
            }
        }
    } catch (const std::exception& e) {
        Log("Exception during copy: " + std::string(e.what()));
    }

    // 4. Cleanup
    Log("Cleaning up...");
    std::error_code ec;
    fs::remove_all(tempDir, ec);
    fs::remove(zipPath, ec);

    // 5. Restart app
    std::string exePath = destDir + "\\MouseShifter.exe";
    Log("Restarting: " + exePath);
    
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    if (CreateProcessA(exePath.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, destDir.c_str(), &si, &pi))
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else
    {
        Log("Failed to restart application, error code: " + std::to_string(GetLastError()));
        MessageBoxA(NULL, "Update applied but failed to restart MouseShifter. Please start it manually.", "Update Complete", MB_OK | MB_ICONINFORMATION);
    }

    Log("--- Updater Finished ---");
    return 0;
}
