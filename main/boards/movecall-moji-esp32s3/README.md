# Movecall Moji (ESP32-S3)

- **Display**: GC9A01 240×240 round SPI LCD
- **Flash**: 16MB
- **PSRAM**: 8MB (Octal)
- **Audio**: ES8311 codec + speaker
- **LED**: GPIO 21

## Build & Flash

```bash
rm -rf build
idf.py -DBOARD=movecall-moji-esp32s3 set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```
