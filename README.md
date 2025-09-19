# 🕹️ MouseShifter

**MouseShifter** allows you to use your mouse as an H-pattern gear shifter in games, providing precise control and an engaging driving experience.

> It supports popular sim racing and driving games like *Euro Truck Simulator 2*, *American Truck Simulator*, *BeamNG.drive*, *Project CARS*, and more.

**⚠ Note:** This doesn’t disable mouse steering — you can still use mouse steering while using MouseShifter! It simply adds gear shifting functionality without interfering with your existing controls.

---
[![⬇️ Download MouseShifter](https://img.shields.io/badge/Download-MouseShifter-blue?style=for-the-badge)](https://github.com/arnofrxdd/MouseShifter/releases/download/Release/MouseShifter.zip)
[![☕ Buy Me a Coffee](https://img.shields.io/badge/Buy_Me_a_Coffee-FFDD00?style=for-the-badge&logo=buy-me-a-coffee&logoColor=000000)](https://buymeacoffee.com/harshitparg)

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

## ⚡ How to Use

1. Download the latest release from GitHub and extract the ZIP file.  

<details>
<summary>⚠️ IMPORTANT</summary>

- **Make sure you have at least 12 buttons set and all axes enabled in vJoy settings, must always be vJoy Device: 1**.

Start -> Search "Configure vJoy"

![vJoyConfigure](ETS2/configurevjoy.png)

</details>

2. Run `MouseShifter.exe` as Administrator.  
3. Position the overlay window where you want it, then press **Tab**.  
4. Press **F12** to temporarily disable the knob.  
5. Launch your game.  
6. Open the control settings and bind your gear controls.  
![Binding](./buttonbindings.gif)  
7. Done — you’re ready to play!  

> ⚠️ **Tip:** If the overlay doesn’t appear or function properly, try setting your game to **borderless mode**.

---






<details>
<summary>📚 Guides: Euro Truck Simulator 2 & American Truck Simulator</summary>

### ⚠️ Important
- **Before proceeding, create a backup of your `controls.sii` file**.  
- **Disable Steam Cloud**
- **Run MouseShifter as Admin**
- **Make sure you have atleast 12 buttons set and all axes enabled in vJoy settings, must always be vJoy Device: 1**.

  Start -> Search "Configure vJoy"
  
![vJoyConfigure](ETS2/configurevjoy.png)

---

### 🚛 Using in Euro Truck Simulator 2 / American Truck Simulator

1. Make sure all gears are bound to vJoy buttons in the app (default bindings are included).
2. Open the app, then launch the game.  
3. Go to **Options → Controls**, and choose **Keyboard + vJoy Device** as the main device.  

![vJoy Select](ETS2/Keyboard+vJoy.png)

4. Scroll to **H-Shifter** and map all gears and togglers.  
![Map H-Shifter](ETS2/H-Shifter%20ETS2.png)
5. Shift gears with your mouse while holding **Right Click** to look around.
---

### 🖱 Using Mouse Steering

1. Enable **Mouse Steering** in the app.  
2. Choose **H-Shifter Mouse Device** and **Mouse Steering Device**.  
3. Within the game, go to **Options → Controls**, and:  
   - ❌ **Do not** enable Keyboard + Mouse Steering.  
4. Bind the steering axis, and optionally bind the acceleration or brake axes for mouse throttle or brake.
![MouseSteer](ETS2/MouseSteer.png)

> ⚠️ **Warning:** Press **F11** to temporarily turn off mouse steering before pausing or accessing in-game menus.  
> ℹ️ **Tip:** If you want to use **dual mouse steering + H-Shifter**, see the [Dual Mouse Setup](#dual-mouse-setup) below.
---

### 🛞 Using Wheel or Pedals

1. In the game, go to **Options → Controls**.  
2. Keep **Keyboard + vJoy Device** selected as your primary device.  
3. Pick your **wheel or pedals** as an additional device.  
4. Bind the steering, throttle, brake, or clutch as needed.  
5. That’s it — you’re ready to drive!

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

<details>
<summary>🖱️ Mouse Steering Guide</summary>

### **Single Mouse Setup**

1. Open the **MouseShifter** app.  
2. Enable **Mouse Steering**.  
3. Select **H-Shifter Mouse Device**: choose your USB mouse for H-Shifter.  
4. Select **Mouse Steering Device**: select the same mouse as the H-Shifter.  
5. Your H-Shifter will now be temporarily disabled; hold down the **Activate Knob Key** to use H-Shifter.  
6. Open your game and go to **Control Options**.  
   - **Do NOT enable Mouse Steering** if your game enables it by default.  
7. Bind your steering:
   - For key bindings: select left/right and move your mouse accordingly.  
   - For steering axes bindings: select the axis and move your mouse (method may vary per game).  
8. If you haven’t already, bind H-Shifter buttons.  
9. Done! You can now play with mouse steering while holding the **Activate Knob Key** to use H-Shifter.  

> ⚠️ **Note:** Always press **F11** to temporarily disable mouse steering before pausing the game or accessing menus.

---

### **Dual Mouse Setup**

1. Open the **MouseShifter** app.  
2. Enable **Mouse Steering**.  
3. Select **H-Shifter Mouse Device**: choose the mouse you want for H-Shifter.  
4. Select **Mouse Steering Device**: choose your second mouse for steering.  
5. Open your game and go to **Control Options**.  
   - **Do NOT enable Mouse Steering** if your game enables it by default.  
6. Bind your steering(method may vary per game):
   - For key/button bindings: select left/right and move your steering mouse accordingly.  
   - For steering axes bindings: select the axis and move your mouse.   
7. If you haven’t already, bind H-Shifter buttons.  
8. Done! You can now play with mouse steering enabled while using the second mouse for H-Shifter.  

> ⚠️ **Note:** Always press **F11** to temporarily disable mouse steering before pausing the game or accessing menus.

</details>

---

## ❓ FAQ  

**Q: Why do I need vJoy?**  
A: Most games do not recognize mouse movements or keys as H-shifter inputs. vJoy serves as a **virtual joystick**, so your mouse or controller inputs can be mapped as proper gear shifts in the game.  

**Q: Why isn’t my MouseShifter knob moving while in-game?**  
A: ✅ **Run MouseShifter as administrator.**  

**Q: When will Linux be supported?**  
A: 🐧 Not very soon – Linux support is not on the roadmap at the moment, but it’s something to keep in mind for the future.  

**Q: Why isn’t my reverse gear binding?**  
A: 🔄 Make sure you have **at least 12 buttons** set and **all axes enabled** in vJoy settings.  
It must always be on **vJoy Device: 1**.  
To fix:  
- Open Start → Search **“Configure vJoy”**  
- Set **Buttons: 12+**  
- Enable **all axes**  
- Apply changes
  
### Q: Will the source code be available?  
**A:** 📂 Yes – the source code will be released soon. I’m currently refactoring it to make it cleaner, easier to understand, and more developer-friendly for anyone who wants to tweak or contribute.  

---

## 📬 Feedback & Feature Requests

You might encounter some bugs or issues as this tool continues to develop.  
I’d appreciate your feedback—whether it’s reporting bugs, asking for new features, or suggesting improvements!

**The best way to contact me:**  
➡️ [GitHub Issues](https://github.com/arnofrxdd/MouseShifter/issues) – Please create an issue for bugs, feature requests, or feedback.

---

## 📝 Notes

- ⚙️ Controller support is experimental; XInput works natively, but PlayStation controllers require DS4Windows.  
- 🎮 Built-in vJoy ensures smooth compatibility with games that support virtual joysticks.
