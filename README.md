# 🕹️ MouseShifter

**MouseShifter** allows you to use your mouse as an H-pattern gear shifter in games, providing precise control and an engaging driving experience.

> It supports popular sim racing and driving games like *Euro Truck Simulator 2*, *American Truck Simulator*, *BeamNG.drive*, *Project CARS*, and more.

**⚠ Note:** This doesn’t disable mouse steering — you can still use mouse steering while using MouseShifter! It simply adds gear shifting functionality without interfering with your existing controls.

---
[![⬇️ Download MouseShifter](https://img.shields.io/badge/Download-MouseShifter-blue?style=for-the-badge)](https://github.com/arnofrxdd/MouseShifter/releases/download/Release/MouseShifter.zip)

## ✨ Features

- 🎛️ **Mouse-Based H-Shifter**  
- 🗂️ **Customizable Gear Layouts**  
- ⚙️ **12/16 Gearbox Support**  
- 🎮 **Built-in vJoy Support**  
- 🕹️ **Experimental Controller Support**
- 🛞 **Dual Mouse Control (Steering + Shifting)**
- 🎨 **Fully Customizable Controls and Layout**

---

![MouseShifter](MouseShifter.png)

---

## 🖥️ Requirements

- ✅ [vJoy driver](https://sourceforge.net/projects/vjoystick/) – This virtual joystick driver is necessary for input mapping.  
- ✅ [DS4Windows](https://github.com/ryochan7/ds4windows/releases) – An optional tool for PlayStation controllers to enable XInput support.

---

## 🚀 Installation

1. Download the latest version from GitHub.  
2. Run `MouseShifter.exe`.  
3. Set up your gear layout, knob, and sensitivity settings.  
4. Move the window where you want the overlay and press **Tab**.  
5. Start your game.
6. Bind Gear controls.
7. Use the overlay to shift gears with your mouse.

> ⚠️ **Note:** If the overlay doesn’t show up or work correctly, try switching your game to **borderless mode**.

---

<details>
<summary>📚 Guides: Euro Truck Simulator 2 & American Truck Simulator</summary>

### ⚠️ Important
- **Before proceeding, create a backup of your `controls.sii` file**.  
- **Disable Steam Cloud**
- **Run MouseShifter as Admin**
---

### 🚛 Using in Euro Truck Simulator 2 / American Truck Simulator

1. Make sure all gears are bound to vJoy buttons in the app (default bindings are included).
2. Open the app, then launch the game.  
3. Go to **Options → Controls**, and choose **Keyboard + vJoy Device** as the main device.  
4. Scroll to **H-Shifter** and map all gears and togglers.  
5. Shift gears with your mouse while holding **Right Click** to look around.

---

### 🖱 Using Mouse Steering

1. Enable **Mouse Steering** in the app.  
2. Choose **H-Shifter Mouse Device** and **Mouse Steering Device**.  
3. Within the game, go to **Options → Controls**, and:  
   - ❌ **Do not** enable Keyboard + Mouse Steering.  
4. Bind the steering axis, and optionally bind the acceleration or brake axes for mouse throttle or brake.

> ⚠️ **Warning:** Press **F11** to temporarily turn off mouse steering before pausing or accessing in-game menus.

---

### 🎮 Using Experimental Controller Support

1. Enable **Controller** in the app.
2. DO NOT SELECT Your controller yet as an additional device.
3. Bind Look Axis in the game if needed.  
4. Select your **XInput controller** as an additional device.  
5. Bind H-Shifter togglers, buttons, acceleration, and brake axes.  
6. Use the **Assist Knob button** to shift gears.

---

![ETS2 Setup](ETS2.gif)

</details>


---

## ❓ FAQ

**Q: Why do I need vJoy?**  
A: Most games do not recognize mouse movements or keys as H-shifter inputs. vJoy serves as a **virtual joystick**, so your mouse or controller inputs can be mapped as proper gear shifts in the game.

**Q: Why isn’t my MouseShifter knob moving while in-game?**  
A: ✅ **Run MouseShifter as administrator.**

**Q: When will Linux be supported?**  
A: 🐧 Not very soon – Linux support is not on the roadmap at the moment, but it’s something to keep in mind for the future.


---

## 📬 Feedback & Feature Requests

You might encounter some bugs or issues as this tool continues to develop.  
I’d appreciate your feedback—whether it’s reporting bugs, asking for new features, or suggesting improvements!

**The best way to contact me:**  
➡️ [GitHub Issues](https://github.com/yourusername/MouseShifter/issues) – Please create an issue for bugs, feature requests, or feedback.

---

## 📝 Notes

- ⚙️ Controller support is experimental; XInput works natively, but PlayStation controllers require DS4Windows.  
- 🎮 Built-in vJoy ensures smooth compatibility with games that support virtual joysticks.
