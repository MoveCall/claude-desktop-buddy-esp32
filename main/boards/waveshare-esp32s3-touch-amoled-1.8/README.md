# Waveshare ESP32-S3-Touch-AMOLED-1.8 (ESP32-S3)

- **Display**: SH8601 368×448 QSPI AMOLED
- **Flash**: 16MB
- **PSRAM**: 8MB (Octal)
- **Audio**: ES8311 codec (shared I2C with PMIC)
- **Backlight**: AMOLED panel command (0x51)

## Build & Flash

```bash
rm -rf build
idf.py -DBOARD=waveshare-esp32s3-touch-amoled-1.8 set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```
