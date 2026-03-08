# Example 1: Hello LED (Blinky)

The classic embedded "Hello World" — blinks an LED on a GPIO pin using Zephyr's GPIO driver, kernel sleep, structured logging, and Kconfig.

---

## What it teaches

| Concept | Description |
|---|---|
| **GPIO driver** | Configure and toggle a hardware pin using `gpio_pin_configure` and `gpio_pin_toggle` |
| **Kernel sleep** | `k_msleep()` yields the CPU to other threads instead of busy-waiting |
| **Kconfig** | Define and use build-time options: blink interval, GPIO pin, log enable |
| **Logging** | Zephyr's structured logging subsystem (`LOG_INF`, `LOG_ERR`) |
| **Main thread** | The entry point Zephyr creates automatically after boot |

---

## File structure

```
hello_led/
  CMakeLists.txt     # Build system configuration
  Kconfig            # Application-specific configuration options
  prj.conf           # Default values for Kconfig options
  src/
    main.c           # Application code (read this first!)
```

---

## Kconfig options

These options are defined in `Kconfig` and set in `prj.conf`. All of them can be overridden at build time without modifying any file.

| Option | Default | Description |
|---|---|---|
| `CONFIG_HELLO_LED_BLINK_INTERVAL_MS` | `1000` | Time in ms between LED toggles |
| `CONFIG_HELLO_LED_GPIO_PIN` | `14` | GPIO pin where the LED is connected |
| `CONFIG_HELLO_LED_LOG_ENABLED` | `y` | Print LED state to console on each toggle |

---

## Three ways to configure

### 1. Edit `prj.conf` (persistent change)

Open `prj.conf` and change any value. The change is saved for all future builds.

```
# Make the LED blink 5x faster
CONFIG_HELLO_LED_BLINK_INTERVAL_MS=200

# Use a different GPIO pin
CONFIG_HELLO_LED_GPIO_PIN=2
```

Then rebuild (no need to clean):

```bash
west build --build-dir examples/hello_led/build
```

### 2. Pass an extra config file at build time (one-off override)

Use `-- -DEXTRA_CONF_FILE=` to apply an additional `.conf` file on top of `prj.conf`, without touching it. Useful for trying a different configuration or keeping sensitive values (credentials, API keys) out of version control.

Create a file, e.g. `examples/hello_led/fast_blink.conf`:

```
CONFIG_HELLO_LED_BLINK_INTERVAL_MS=100
CONFIG_HELLO_LED_GPIO_PIN=2
```

Then build with it:

```bash
west build -p -b <your_board> examples/hello_led \
    --build-dir examples/hello_led/build \
    -- -DEXTRA_CONF_FILE="fast_blink.conf"
```

> The `--` separates `west build` arguments from raw CMake arguments. Everything after `--` is passed directly to CMake.

### 3. Interactive menu (`menuconfig`)

Explore and change **all** available options (Zephyr kernel + drivers + your app) through a terminal UI:

```bash
# Build first (generates the config), then open the menu
west build -p -b <your_board> examples/hello_led \
    --build-dir examples/hello_led/build

west build --build-dir examples/hello_led/build -t menuconfig
```

Use arrow keys to navigate, `Space` to toggle, `Enter` to enter a submenu. Your app options appear under **"Hello LED Example Configuration"**. Save with `S`, exit with `Q`. Then rebuild:

```bash
west build --build-dir examples/hello_led/build
```

---

## Build

Always use `-p` (pristine/clean) when switching boards or after significant changes. Always specify `--build-dir` to keep build artifacts inside the example folder. **Replace `-b` with your board identifier.**

```bash
# Build for ESP32 Wrover Kit
west build -p -b esp_wrover_kit/esp32/procpu examples/hello_led \
    --build-dir examples/hello_led/build

# Build for Nordic nRF52840 DK
west build -p -b nrf52840dk/nrf52840 examples/hello_led \
    --build-dir examples/hello_led/build

# Build for STM32 Nucleo F401RE
west build -p -b nucleo_f401re examples/hello_led \
    --build-dir examples/hello_led/build

# Build with an extra config file on top of prj.conf
west build -p -b <your_board> examples/hello_led \
    --build-dir examples/hello_led/build \
    -- -DEXTRA_CONF_FILE="fast_blink.conf"

# Incremental rebuild (no clean, after a small code or config change)
west build --build-dir examples/hello_led/build
```

> **Tip:** Adjust `CONFIG_HELLO_LED_GPIO_PIN` in `prj.conf` to match the LED pin on your board.

---

## Flash

Always specify `--build-dir` so west knows which binary to flash. West auto-detects the correct flash runner for your board.

```bash
# Generic (works for most boards: Nordic, STM32, etc.)
west flash --build-dir examples/hello_led/build

# ESP32 — use --esp-device to specify the serial port
west flash --build-dir examples/hello_led/build --esp-device /dev/ttyUSB0
```

> Find your device port with: `ls /dev/ttyUSB* /dev/ttyACM*`

---

## Monitor serial output

Choose the monitoring method that matches your board:

```bash
# ── Generic (works for any board) ─────────────────────────────────────────
minicom -D /dev/ttyACM0 -b 115200
# or: screen /dev/ttyACM0 115200
# or: picocom -b 115200 /dev/ttyACM0

# ── ESP32 (requires esptool: pip install esptool) ─────────────────────────
cd examples/hello_led/build && west espressif monitor -p /dev/ttyUSB0
```

Expected output:

```
[00:00:00.123,000] <inf> hello_led: Hello LED started - blinking GPIO14 every 1000 ms
[00:00:01.123,000] <inf> hello_led: LED state: OFF
[00:00:02.123,000] <inf> hello_led: LED state: ON
...
```
