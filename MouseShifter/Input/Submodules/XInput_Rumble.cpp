#include <mutex>
#include <chrono>

std::mutex rumbleMutex;
std::chrono::steady_clock::time_point lastRumbleTime;

void SafeRumble(int left, int right, int durationMs)
{
    std::lock_guard<std::mutex> lock(rumbleMutex);
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRumbleTime).count();

    if (elapsed < 60)
        return; // debounce rumble spam

    for (auto& g : g_gamepads)
    {
        if (g.controller)
        {
            SDL_GameControllerRumble(g.controller, 0, 0, 0);
            SDL_HapticStopAll(g.haptic);
        }

        if (g.haptic)
        {
            SDL_HapticRumblePlay(g.haptic, (std::max)(left, right) / 65535.0f, durationMs);
        }
        else if (g.controller)
        {
            SDL_GameControllerRumble(g.controller, left, right, durationMs);
        }
    }

    lastRumbleTime = now;
}
