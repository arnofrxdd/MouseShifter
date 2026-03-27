bool throttleBrakeThreadRunning = false;
HANDLE throttleBrakeThread = NULL;
DWORD throttleBrakeThreadId = 0;
LONG joyZ = 0;
LONG joyRz = 0;
float throttleSensitivity = 8.5f;
float brakeSensitivity = 8.5f;
float releaseSpeed = 8.5f;

// Thread synchronization
CRITICAL_SECTION throttleBrakeCS;

