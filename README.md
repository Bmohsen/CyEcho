# ⌨️ CyEcho
**A minimalist, high-performance mechanical keyboard sound emulator.**

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Platform: Windows](https://img.shields.io/badge/Platform-Windows-0078d4.svg)]()
[![C++20](https://img.shields.io/badge/C++-20-blue.svg)]()
[![Size: < 5MB](https://img.shields.io/badge/Binary-Tiny-brightgreen.svg)]()

CyEcho brings the satisfying acoustics of high-end mechanical switches to any keyboard. Built with a focus on **zero latency**, **minimal binary footprint**, and a **distraction-free TUI**.

---

## ✨ Features

* **⚡ Ultra-Low Latency:** Powered by `libuiohook` and `miniaudio` for near-instant response.
* **📦 Tiny Footprint:** Compiled to a single small executable with minimal RAM usage.
* **📟 Elegant TUI:** A sleek Terminal User Interface for configuration—no heavy GUI frameworks.
* **☁️ Stealth Mode:** Minimizes to the System Tray with zero taskbar presence.
* **🎧 Custom Profiles:** Support for Cherry MX, IBM Model M, Topre, and custom user-made packs.
---

## 🚀 Quick Start

### Installation
Download the latest `CyEcho.exe` from the [Releases](https://github.com/bmohsen/cyecho/releases) page.

### Usage
1. Run `CyEcho.exe`.
2. Use **Arrow Keys** to navigate sound profiles.
3. Use the **Volume Slider** to adjust output.
4. Press **'m'** to hide the app to the System Tray (Tool Window mode).
5. Double-click the **Tray Icon** to bring the settings back.

---

## 🛠️ Building from Source

### Prerequisites
* **Visual Studio 2022** (MSVC)
* **CMake 3.20+**

### Build Steps
```bash
# Clone the repository
git clone [https://github.com/bmohsen/cyecho.git](https://github.com/bmohsen/cyecho.git)
cd cyecho

# Configure and Build
cmake -B build -S .
cmake --build build --config Release
```
---
## 📂 Project Structure
CyEcho/
├── src/
│   ├── main.cpp          # TUI, System Tray, & Window Styles
│   ├── KeyCapture.cpp    # Global Input Hooking & State Tracking
│   └── Play.cpp          # Audio Engine & Voice Pooling (miniaudio)
├── resources/            # Sound Profiles (JSON + OGG/WAV)
├── include/              # Header files
└── cmake/                # Build configurations

---
## 🤝 Contributing
#### Contributions are welcome! Whether it's a bug fix, a new feature, or a new sound pack, feel free to open a Pull Request.

   1. Fork the Project.

   2. Create your Feature Branch (git checkout -b feature/AmazingFeature).

   3. Commit your Changes (git commit -m 'Add some AmazingFeature').

   4. Push to the Branch (git push origin feature/AmazingFeature).

   5. Open a Pull Request.

--- 
## 🙏 Acknowledgments

   - miniaudio - Single-header audio library.

   - FTXUI - Functional Terminal User Interface.

   - libuiohook - Cross-platform global hook library.