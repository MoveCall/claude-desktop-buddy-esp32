# Movecall MoHi (ESP32-C3)

- **Chip**: ESP32-C3
- **Display**: ST7789 160×80 SPI LCD (landscape)
- **Flash**: 4MB
- **Audio**: PDM speaker (GPIO 6/7) + PA (GPIO 3)
- **LED**: WS2812 × 4 (GPIO 8)
- **Button**: 1 (BOOT, GPIO 9)

## Build & Flash

```bash
rm -rf build
idf.py -DBOARD=movecall-mohi-esp32c3 set-target esp32c3
idf.py build
idf.py -p PORT flash monitor
```

## Controls

| Action | No approval | Approval pending |
|--------|------------|-----------------|
| **Click** | Next page | Approve |
| **Long press** | Deny | Deny |
| **Double click** | Switch pet | Switch pet |
| **Triple click** | Demo mode | — |

## Notes

Based on [esp-hi](https://github.com/78/xiaozhi-esp32/tree/main/main/boards/esp-hi) pin layout.
Open hardware: [oshwhub.com/movecall/mohi-reproduction-ai-dialogue-ro](https://oshwhub.com/movecall/mohi-reproduction-ai-dialogue-ro)
