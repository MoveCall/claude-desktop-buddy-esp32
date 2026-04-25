# Espressif ESP-BOX-3 (ESP32-S3)

- **Display**: ILI9341 320×240 SPI LCD
- **Flash**: 16MB
- **PSRAM**: 8MB (Octal)
- **Audio**: ES8311 codec + speaker

## Build & Flash

```bash
rm -rf build
idf.py -DBOARD=esp-box-3 set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```
