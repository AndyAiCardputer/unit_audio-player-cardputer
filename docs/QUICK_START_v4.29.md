# 🚀 Quick Start Guide - Audio Player v4.29.0

## 🎉 What's New?

### 💿 Jamaican Vinyl Splash (Splash Screen)
When you turn on the player, you'll see a cool vinyl record animation!

### 💚 Matrix Rain (3rd visualization)
Now you have 3 visualization modes, including the classic Matrix effect!

---

## ⚡ Quick Start

### 1. Loading the code

```bash
# In Arduino IDE:
1. Open audio_player_unit.ino
2. Select board: M5Stack Cardputer
3. Click Upload (➡️)
```

### 2. First Launch

On startup you'll see:
```
┌─────────────────────────────────┐
│  💿 Jamaican Vinyl Splash       │
│                                 │
│  🔴 Album cover                 │
│  🟡 Record slides out           │
│  🟢 Spinning with notes 🎵      │
│  📝 "AndyShuffle v4.29"         │
│                                 │
│  Press any key to skip ⏭️       │
└─────────────────────────────────┘
```

**Want to skip?** Press any button!

---

## 🎮 Controls

### Basic buttons:

| Button | Action |
|--------|--------|
| **BtnA** (1x) | Play/Pause ▶️⏸️ |
| **BtnA** (2x) | Next track ⏭️ |
| **BtnA** (3x) | Previous track ⏮️ |
| **+** | Volume up 🔊 |
| **-** | Volume down 🔉 |
| **V** | Visualization 🎨 |
| **I** | Information ℹ️ |

### Visualization (V):

Press **V** to cycle:

```
1st time → 📊 Bars (columns)
           ║ ║ ║ ║ ║ ║
           
2nd time → 🌊 Waves (waveforms)
           ～～～～～～
           
3rd time → 💚 MATRIX RAIN!
           01♪10♫01
           
4th time → Exit to main screen
```

### Matrix Rain features:

- 🟢 Classic green color
- ♪♫ Musical symbols (every ~20th)
- 🎶 Faster when playing
- ⏸️ Slower when paused
- **ESC** → exit

---

## 🎨 Jamaican Vinyl Details

### Color Scheme 🇯🇲

Splash screen uses Jamaican colors:
- 🔴 **Red** - top stripe
- 🟡 **Yellow** - middle stripe
- 🟢 **Green** - bottom stripe

### Animation:

```
Stage 1 (0.6 sec):     Stage 2 (0.7 sec):
┌─────────┐           ┌─────────┐  ●
│         │           │         │ ╱│╲
│  Andy   │  →        │  Andy   │  ● 
│ Shuffle │           │ Shuffle │ ╱│╲
└─────────┘           └─────────┘  ●

Stage 3 (0.9 sec):     Stage 4 (0.6 sec):
      ●                     ●
     ╱│╲                   ╱│╲
  🎵  ●  🎵         🎵  ●  🎵
     ╱│╲   (spin)         ╱│╲
      ●                     ●
                    
                    AndyShuffle v4.29
```

**Total:** ~3 seconds of cool animation!

---

## 💡 Tips & Tricks

### Splash Screen:
- Shows only **once** per power cycle
- Skip: press **any button**
- Unique reggae vibe! 🎸

### Matrix Rain:
- Looks better when music is playing
- Characters fall faster with music
- Hunt for musical notes ♪ ♫ between characters!

### Performance:
- Matrix updates every **80ms** (~12 FPS)
- Low CPU load
- Smooth animation

---

## 🐛 Troubleshooting

### Splash screen not showing?
- Check `splashShown` variable
- It resets only on reboot

### Matrix lagging?
- Normal! That's the "Matrix" effect
- If too laggy, increase `matrixUpdateInterval`

### No musical symbols?
- They're rare (every ~20th)
- Wait a bit - they'll appear!

---

## 📊 Mode Comparison

| Mode | Style | Load | Coolness |
|------|-------|------|----------|
| **Bars** | Classic | Low | ⭐⭐⭐⭐ |
| **Waves** | Smooth | Medium | ⭐⭐⭐⭐ |
| **Matrix** | Hacker | Low | ⭐⭐⭐⭐⭐ |

---

## 🎯 Combinations

### For demonstration:
1. Power on → **Jamaican Vinyl** ✨
2. Press **V** → **Bars**
3. Press **V** → **Waves**
4. Press **V** → **MATRIX!** 💚
5. Press **I** → Info screen

### For usage:
1. **Any key** → skip splash
2. Start music → **BtnA**
3. **V** → Matrix Rain
4. Enjoy! 🎉

---

## 🎵 Music Recommendations

### For Matrix Rain:
- 🎸 Dubstep - energetic drops
- 🥁 Drum & Bass - fast rhythms
- 🎹 Electronic - digital vibe
- 🎺 Reggae - for Jamaican theme!

### For Jamaican Vinyl:
- 🇯🇲 Bob Marley - perfect!
- 🎵 Reggae classics
- 🌴 Tropical vibes

---

## 📝 Notes

### Versions:
- **v4.28.6** → Fixed text position
- **v4.29.0** → Jamaican Vinyl + Matrix Rain!

### Next update:
Possible:
- More Matrix color schemes
- Additional splash screens
- Real audio synchronization

---

## 🚀 Done!

Now you have the coolest Audio Player for Cardputer!

**Features:**
- ✅ Jamaican Vinyl splash screen 💿
- ✅ 3 visualization modes 🎨
- ✅ Matrix Rain effect 💚
- ✅ Musical notes ♪♫
- ✅ Skippable splash
- ✅ Reggae vibe 🇯🇲

---

**Enjoy!** 🎉🎵💚

Made by **Andy+AI** with ❤️

*Press V to unleash the Matrix!* ⚡

