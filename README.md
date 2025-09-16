# 🕹️ MouseShifter

**MouseShifter** lets you use your mouse as an H-pattern gear shifter in games, giving you precise control and an immersive driving experience.

> Supports popular sim racing and driving games such as Euro Truck Simulator 2, American Truck Simulator, BeamNG.drive, Project CARS, and more.

---

## ✨ Features

- 🎛️ **Mouse-Based H-Shifter**  
- 🗂️ **Customizable Gear Layouts**  
- ⚙️ **12/16 Gearbox Support**  
- 🎮 **Built-in vJoy Support**  
- 🕹️ **Experimental Controller Support**  
- 🎨 **Fully Customizable Controls and Layout**

---

![MouseShifter](MouseShifter.png)

---

## 🖥️ Requirements

- ✅ [vJoy driver](https://sourceforge.net/projects/vjoystick/) – Virtual joystick driver required for input mapping.  
- ✅ [DS4Windows](https://github.com/ryochan7/ds4windows/releases) – Optional tool for PlayStation controllers to enable XInput support.

---

## 🚀 Installation

1. Download the latest release from GitHub.  
2. Run `MouseShifter.exe`.  
3. Configure your gear layout, knob, and sensitivity settings.  
4. Move the window to where you want the overlay and press **Tab**.  
5. Launch your game.  
6. Use the overlay to shift gears with your mouse.  

> ⚠️ **Note:** If the overlay doesn’t appear or function properly, try switching your game to borderless mode.

---

<details>
<summary>📚 Euro Truck Simulator 2 / American Truck Simulator Tutorials (click to expand)</summary>
   
### Using in Euro Truck Simulator 2 / American Truck Simulator

1. Ensure all gears are bound to vJoy buttons in the app (default bindings included).  
2. Open the app, then launch the game (Right-click temporarily enables looking around).  
3. Go to **Options → Controls**, select **Keyboard + vJoy Device** as the primary device.  
4. Scroll down to **H-Shifter** and map all gears and togglers.  
5. Shift gears using your mouse while holding **Right Click** to look around.

### Using Mouse Steering

1. Enable **Mouse Steering** in the app.  
2. Select **H-Shifter Mouse Device** and **Mouse Steering Device**.  
3. In the game, go to **Options → Controls**.  
   - **DO NOT** enable Keyboard + Mouse Steering.  
4. Bind the steering axis and optionally acceleration/brake axes if you want mouse throttle or brake.  
> ⚠️ **Warning:** Press **F11** to temporarily disable mouse steering before pausing or accessing in-game menus.

### Using Experimental Controller Support

1. Enable **Controller** in the app.  
2. Bind Look Axis in the game if needed.  
3. Select your **XInput controller** as an additional device.  
4. Bind H-Shifter togglers, buttons, acceleration, and brake axes.  
5. Use the **Assist Knob button** to shift gears.
![ETS2 Setup](ETS2.gif)

</details>

---

## ❓ FAQ

**Q: Why do I need vJoy?**  
A: Most games do not natively recognize mouse movements or keys as H-shifter inputs. vJoy acts as a **virtual joystick**, allowing your mouse or controller inputs to be mapped as proper gear shifts in the game.

**Q: Why isn’t my MouseShifter knob moving while in-game?**  
A: **Run MouseShifter as administrator.**

---

## 📬 Feedback & Feature Requests

You might expect some bugs or issues as this is an evolving tool.  
I’d love to hear from you—whether it's reporting bugs, requesting new features, or suggesting improvements!

**Best place to reach me:**  
➡️ [GitHub Issues](https://github.com/yourusername/MouseShifter/issues) – Please create an issue for bugs, feature requests, or feedback.  

---

## 📝 Notes

- Controller support is experimental; XInput works natively, PS controllers require DS4Windows.  
- Built-in vJoy ensures smooth compatibility with games that support virtual joysticks.
