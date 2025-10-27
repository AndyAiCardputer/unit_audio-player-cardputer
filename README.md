# 🎵 Audio Player for M5Stack Cardputer

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/platform-ESP32--S3-blue)](https://github.com/m5stack/M5Cardputer)
[![Version](https://img.shields.io/badge/version-v4.29.0-green)](https://github.com/AndyAiCardputer/unit_audio-player-cardputer/releases)

Feature-rich **MP3/WAV Unit Audio Player** for M5Stack Cardputer with stunning visualizations.

---

## ✨ Features

- ✅ **MP3 & WAV playback** from SD card
- ✅ **3 Visualization modes:**
  - 📊 Bars (classic equalizer)
  - 🌊 Waves (smooth waveforms)
  - 💚 **Matrix Rain** (legendary effect!)
- ✅ **Music controls:** Play/Pause, Next/Previous
- ✅ **Volume control** (10 levels)
- ✅ **File browser** for SD card
- ✅ **Battery indicator**
- ✅ **Real-time spectrum analyzer**

---

## 📸 Screenshot
---

## 🎮 Controls

### Basic Controls

| Key | Function |
|-----|----------|
| `BtnA` (1x click) | Play/Pause ▶️⏸️ |
| `BtnA` (2x click) | Next track ⏭️ |
| `BtnA` (3x click) | Previous track ⏮️ |
| `+` | Volume up 🔊 |
| `-` | Volume down 🔉 |
| `V` | Cycle visualizations 🎨 |
| `I` | Show information ℹ️ |
| `ESC` | Exit visualization |

### Visualizations (Press V)

```
1st V → 📊 Bars    (equalizer columns)
2nd V → 🌊 Waves   (waveform animation)
3rd V → 💚 MATRIX! (falling characters)
4th V → Exit to player
```

---

## 📦 Quick Start

### Option 1: Flash Pre-built Binary (Easiest!)

1. Download `4.29.0.ino.bin` from [Releases](https://github.com/AndyAiCardputer/unit_audio-player-cardputer/releases)
2. Connect your M5Stack Cardputer via USB-C
3. Flash using [esptool](https://github.com/espressif/esptool):

```bash
esptool.py --chip esp32s3 --port /dev/ttyUSB0 --baud 921600 write_flash 0x0 4.29.0.ino.bin
```

4. **Done!** The audio player will start automatically. 🎵

---

### Option 2: Build from Source (Arduino IDE)

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Add M5Stack board support:
   - Tools → Board Manager → Search "M5Stack"
   - Install **M5Stack by M5Stack**
3. Install libraries:
   - M5Unified
   - M5GFX
   - ESP8266Audio
4. Open `src/audio_player_unit.ino`
5. Select board: **M5Stack Cardputer**
6. Click **Upload** ➡️

---

## 💾 SD Card Setup

1. Format SD card as **FAT32**
2. Create folder structure:
```
/music/
├── song1.mp3
├── song2.wav
└── ...
```
3. Insert SD card into Cardputer
4. Press `ESC` to browse files!

**Supported formats:** MP3, WAV

---

## 🎨 Visualization Details

### 📊 Bars Mode
Classic equalizer with vertical bars responding to frequencies.
- Fast response
- Clear frequency separation
- Perfect for EDM and electronic music

### 🌊 Waves Mode
Smooth waveform animation synchronized with audio.
- Flowing animation
- Beautiful curves
- Great for ambient and classical music

### 💚 Matrix Rain
Iconic falling characters effect from The Matrix!
- 🟢 Classic green color
- ♪♫ Musical notes hidden in the code
- 🎶 Speed increases with music intensity
- ⏸️ Slows down when paused
- **Hunt for the musical notes!** (~every 20th character)


## 💡 Tips & Tricks

### For Best Experience:
- 💚 **Matrix:** Best with dubstep or drum & bass
- 📊 **Bars:** Perfect for electronic music with strong bass
- 🌊 **Waves:** Beautiful with classical or ambient tracks

### Performance:
- Matrix Rain updates every **80ms** (~12 FPS)
- Low CPU usage even during visualization
- Smooth playback on all formats

### Customization:
Want to change Matrix Rain speed? Edit this in the code:
```cpp
// Matrix Rain update interval (default 80ms = ~12 FPS):
const unsigned long matrixUpdateInterval = 80;
```

---

## 🛠️ Requirements

### Hardware:
- **M5Stack Cardputer** (ESP32-S3)
- **Micro SD Card** (formatted as FAT32)
- **USB-C cable** for programming

### Software (for building from source):
- Arduino IDE 2.0+
- M5Stack board support
- Required libraries (see Option 2 above)

---

## 📚 Documentation

- **[Quick Start Guide](docs/QUICK_START_v4.29.md)** - Get started in minutes
- **[Release Notes](docs/RELEASE_v4.29.0.md)** - Detailed changelog
- **[Full Changelog](docs/CHANGELOG_v4.29.md)** - Version history

---

## 🎯 Tested Music Formats

| Format | Status | Notes |
|--------|--------|-------|
| **MP3** | ✅ Works perfectly | All bitrates supported |
| **WAV** | ✅ Works perfectly | Uncompressed audio |

---

## 🐛 Known Issues

### Splash Screen:
- Shows only **once** per power cycle
- This is intentional! Reboot to see it again

### Matrix Rain:
- May appear "laggy" at times
- This is the authentic Matrix effect! 
- If too slow, increase `matrixUpdateInterval` in code

### SD Card:
- Must be formatted as **FAT32**
- File names should avoid special characters

---

## 🙏 Credits

- **Original concept:** Andy 🎵
- **Matrix Rain implementation:** AI Assistant (Claude)
- **M5Stack Cardputer:** [M5Stack](https://github.com/m5stack)
- **Audio libraries:** ESP8266Audio team
- **Development:** Andy+AI collaboration

Special thanks to the M5Stack community!

---

## 📝 License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

---

## 🤝 Contributing

Contributions are welcome! Ideas for improvements:
- [ ] More color schemes for Matrix (red/blue/rainbow)
- [ ] Additional splash screen themes
- [ ] Save visualization preference
- [ ] Real-time FFT synchronization

---

## 🎉 Enjoy!

**Made with ❤️ by Andy+AI**

🎵 💿 💚 🎶

*Press V to unleash the Matrix!* ⚡

---

**v4.29.0** | October 2025 | AndyShuffle

