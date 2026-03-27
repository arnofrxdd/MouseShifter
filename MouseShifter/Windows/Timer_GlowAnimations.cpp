        // === UPDATE KEYBIND GLOW ANIMATIONS (ALWAYS PROCESS) ===
        bool anyGlowAnimationChanged = false;
        DWORD currentTime = GetTickCount();

        for (auto& kv : keybindAnimations) {
            auto& animation = kv.second;
            float oldAlpha = animation.glowAlpha;

            if (animation.isHeld) {
                // Key is held down - immediately go to full brightness and stay there
                animation.glowAlpha = MAX_GLOW_ALPHA;
                animation.isActive = false;
            }
            else {
                // Key is not held - handle fade in/out
                if (animation.isActive) {
                    // Fade in
                    float elapsed = (float)(currentTime - animation.activationTime);
                    animation.glowAlpha = min(MAX_GLOW_ALPHA, elapsed / GLOW_FADE_IN_TIME);

                    if (animation.glowAlpha >= MAX_GLOW_ALPHA) {
                        animation.glowAlpha = MAX_GLOW_ALPHA;
                        animation.isActive = false;
                        animation.activationTime = currentTime;
                    }
                }
                else {
                    // Fade out
                    float elapsed = (float)(currentTime - animation.activationTime);
                    if (elapsed < GLOW_FADE_OUT_TIME) {
                        animation.glowAlpha = max(0.0f, MAX_GLOW_ALPHA - (elapsed / GLOW_FADE_OUT_TIME));
                    }
                    else {
                        animation.glowAlpha = 0.0f;
                    }
                }
            }

            if (abs(animation.glowAlpha - oldAlpha) > 0.01f) {
                anyGlowAnimationChanged = true;
            }
        }

        // Clean up completed animations
        std::vector<std::string> toRemove;
        for (auto& kv : keybindAnimations) {
            if (!kv.second.isHeld && !kv.second.isActive && kv.second.glowAlpha <= 0.0f) {
                toRemove.push_back(kv.first);
            }
        }
        for (auto& key : toRemove) {
            keybindAnimations.erase(key);
        }

