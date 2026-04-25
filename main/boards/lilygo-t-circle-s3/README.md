# LILYGO T-Circle-S3 (ESP32-S3)

- **Display**: GC9D01N 160×160 round SPI LCD
- **Flash**: 16MB
- **PSRAM**: 8MB (Octal)
- **Audio**: MAX98357A I2S amplifier

## Build & Flash

```bash
rm -rf build
idf.py -DBOARD=lilygo-t-circle-s3 set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```
