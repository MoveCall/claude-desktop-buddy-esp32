# Magiclick 2P5 (ESP32-S3)

- **Display**: GC9107/ST7735 128×128 SPI LCD
- **Flash**: 16MB
- **PSRAM**: 8MB (Octal)
- **Audio**: ES8311 codec + PA
- **LED**: WS2812 × 2 (GPIO 38, power GPIO 39)
- **Button**: 1 (Main, GPIO 21)

## Controls

| Action | No approval | Approval pending |
|--------|------------|-----------------|
| **Click** | Next page | Approve |
| **Long press** | Deny | Deny |
| **Double click** | Switch pet | Switch pet |
| **Triple click** | Demo mode | — |

## Build & Flash

```bash
rm -rf build
idf.py -DBOARD=magiclick-2p5 set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```
