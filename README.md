# claude-desktop-buddy-esp32

<p align="center">
  <img src="docs/hero_v1.png" alt="Claude Desktop Buddy for ESP32" width="960">
</p>

<p align="center">
  <img src="https://img.shields.io/badge/ESP--IDF-v5.5.2+-blue">
  <img src="https://img.shields.io/badge/license-MIT-green">
  <img src="https://img.shields.io/badge/BLE-Nordic%20UART-purple">
  <img src="https://img.shields.io/badge/pets-18%20species-orange">
</p>

ESP32 companion device for Claude Desktop over BLE. Approve permissions, view session status, and interact with a desk pet from hardware. ESP-IDF based, with [xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) hardware ecosystem compatibility.

## Features

- **Live session status** — see how many Claude Cowork/Code sessions are running, waiting, or completed
- **Permission approval** — approve or deny tool calls right from the device button
- **18 ASCII desk pets** — cat, duck, penguin, ghost, robot, octopus, blob, capybara, dragon, goose, owl, rabbit, turtle, snail, mushroom, cactus, chonk
- **GIF character support** — push custom GIF characters from Claude Desktop
- **Clock mode** — displays time when connected and idle
- **LED feedback** — blinks on approval requests, flashes on approve/deny
- **Persistent stats** — approvals, denials, token usage, pet selection survive reboot
- **BLE secure pairing** — LE Secure Connections with 6-digit passkey display

## Supported Hardware

| Board | Chip | Display | Adapted | Verified |
|-------|------|---------|---------|----------|
| **movecall-cuican-esp32s3** | ESP32-S3 | GC9A01 240×240 round | ✅ | ✅ |
| **movecall-moji-esp32s3** | ESP32-S3 | GC9A01 240×240 round | ✅ | ✅ |
| **movecall-moji2-esp32c5** | ESP32-C5 | ST77916 360×360 QSPI round | ✅ | ✅ |

More [xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) compatible boards can be added — the board abstraction layer supports 100+ hardware variants. See [HARDWARE.md](docs/HARDWARE.md) for the full compatibility list.

## Quick Start

### Prerequisites

- [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/get-started/) v5.5.2+
- Claude Desktop (Pro or higher, with Cowork enabled)

### Build & Flash

**ESP32-S3 boards (CuiCan, Moji):**

```bash
idf.py set-target esp32s3
idf.py menuconfig  # Claude Desktop Buddy → Board Type → select your board
idf.py build
idf.py -p PORT flash
```

**ESP32-C5 boards (Moji2):**

```bash
idf.py set-target esp32c5
idf.py menuconfig  # Board Type is auto-selected for C5
idf.py build
# Enter boot mode: unplug USB → hold BOOT → plug USB → release BOOT
idf.py -p PORT flash
# Unplug and replug USB normally to start
```

### Pairing with Claude Desktop

1. Enable Developer Mode: **Help → Troubleshooting → Enable Developer Mode**
2. Open **Developer → Open Hardware Buddy…**
3. Click **Connect** and select your device (advertises as `Claude-XXYY`)
4. Enter the 6-digit passkey shown on the device screen
5. Start a **Cowork** session — the device will show live session status

## Controls

| Action | No approval pending | Approval pending |
|--------|-------------------|-----------------|
| **Click** | Switch display mode (pet ↔ info) | Approve |
| **Long press** | — | Deny |
| **Double click** | Switch pet species | Switch pet species |


## BLE Protocol

Implements the [Claude Desktop Hardware Buddy protocol](https://github.com/anthropics/claude-desktop-buddy/blob/main/REFERENCE.md):

- Nordic UART Service (NUS) over BLE
- JSON messages, newline-terminated
- Heartbeat snapshots (sessions, tokens, entries)
- Permission prompt forwarding and response
- GIF character folder push transfer
- LE Secure Connections with MITM protection

## Adding a New Board

See [docs/ADDING_A_BOARD.md](docs/ADDING_A_BOARD.md) for a step-by-step guide.

## Architecture

```
main/
├── boards/                       # Hardware abstraction (from xiaozhi-esp32)
│   ├── common/                   # Board, Button, Backlight, LED
│   ├── movecall-cuican-esp32s3/  # CuiCan (GC9A01)
│   ├── movecall-moji-esp32s3/    # Moji (GC9A01)
│   └── movecall-moji2-esp32c5/   # Moji2 (ST77916 QSPI)
├── display/                      # LVGL display layer
├── buddy/
│   ├── ble/                      # Nordic UART Service (Bluedroid)
│   ├── core/                     # BuddyApp, TamaState, PersonaState, protocol
│   ├── storage/                  # NVS persistence
│   ├── ui/                       # LVGL UI (round display optimized)
│   ├── pet/                      # 17 ASCII species + GIF character
│   └── xfer/                     # BLE file transfer (LittleFS)
└── main.cc
```

## License

[MIT](LICENSE)

## Acknowledgments

- [xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) — board abstraction and hardware ecosystem
- [claude-desktop-buddy](https://github.com/anthropics/claude-desktop-buddy) — BLE protocol specification and desk pet design
- [Anthropic](https://www.anthropic.com) — Claude Desktop Hardware Buddy API
