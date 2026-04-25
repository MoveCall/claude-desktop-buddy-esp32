# Movecall Moji2 (ESP32-C5)

- **Display**: ST77916 360×360 round QSPI LCD
- **Flash**: 16MB
- **PSRAM**: 8MB (Quad)
- **Audio**: ES8311 codec + speaker
- **LED**: GPIO 10

## Build & Flash

```bash
rm -rf build
idf.py -DBOARD=movecall-moji2-esp32c5 set-target esp32c5
idf.py build
# Enter boot mode: unplug USB → hold BOOT → plug USB → release BOOT
idf.py -p PORT flash
# Unplug and replug USB to start
```
