# M5Stack CoreS3 (ESP32-S3)

- **Display**: ILI9341 320×240 SPI LCD
- **Touch**: FT6336 capacitive (I2C 0x38)
- **Flash**: 16MB
- **PSRAM**: 8MB (Quad)
- **Audio**: AW88298 I2S amplifier
- **Backlight**: AXP2101 PMIC controlled
- **Button**: No physical buttons — all interaction via touch screen

## Controls

| Action | How |
|--------|-----|
| **Approve** | Tap green "Approve" button (shown when pending) |
| **Deny** | Tap red "Deny" button (shown when pending) |
| **Switch pet** | Tap center of screen |
| **Switch page** | Swipe left/right |

## Build & Flash

```bash
rm -rf build
idf.py -DBOARD=m5stack-core-s3 set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```
