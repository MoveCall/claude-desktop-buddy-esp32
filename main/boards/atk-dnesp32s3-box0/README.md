# ATK-DNESP32S3-BOX0 (Alientek, ESP32-S3)

- **Display**: ST7789 240×240 SPI LCD
- **Flash**: 16MB
- **PSRAM**: 8MB (Octal)
- **Audio**: ES8311 codec (no PA, no MCLK)
- **Buttons**: 3 (Left / Middle / Right)

## Controls

| Button | Normal | Approval pending | Settings |
|--------|--------|-----------------|----------|
| **Middle** | Next page | — | Next item |
| **Middle hold** | Open settings | — | — |
| **Right** | Switch pet | Approve | Toggle/Execute |
| **Left** | Demo mode | Deny | Exit settings |

Screen wakes on any button press without triggering an action.

## Build & Flash

```bash
rm -rf build
idf.py -DBOARD=atk-dnesp32s3-box0 set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```
