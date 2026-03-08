# Example 2: Multithreaded LEDs

Two LEDs blink independently, each driven by its own RTOS thread. This example introduces the core multithreading and synchronization concepts of Zephyr.

---

## What it teaches

| Concept | Description |
|---|---|
| **Threads** | Create independent execution contexts with `K_THREAD_DEFINE` |
| **Priorities** | The scheduler always runs the highest-priority ready thread (lower number = higher priority) |
| **Semaphores** | Synchronize thread startup with `K_SEM_DEFINE`, `k_sem_give`, `k_sem_take` |
| **Stack sizes** | Each thread needs its own stack — too small crashes, too big wastes RAM |
| **Kconfig** | Configure stack size, priorities, GPIO pins, and intervals without touching code |

---

## Architecture

```
+----------------+     k_sem_give()     +----------------+
|  Main Thread   | ------------------> |  LED A Thread  |
|                | ---+                 |  GPIO14, 500ms |
+----------------+    |                 |  Priority: 5   |
                      |                 +----------------+
                      |  k_sem_give()
                      +--------------> +----------------+
                                       |  LED B Thread  |
                                       |  GPIO18, 1000ms|
                                       |  Priority: 7   |
                                       +----------------+
```

**Startup sequence:**
1. Zephyr creates all three threads at boot (`K_THREAD_DEFINE` is static)
2. LED A and LED B threads block on a semaphore (`k_sem_take` — zero CPU usage while waiting)
3. Main thread logs startup info, then signals both threads via `k_sem_give` (called twice)
4. Both LED threads unblock and run independently forever, each at its own interval

---

## File structure

```
multithreads_hello_leds/
  CMakeLists.txt          # Build system configuration
  Kconfig                 # Application-specific configuration options
  prj.conf                # Default values for Kconfig options
  src/
    main.c                # Thread definitions (K_THREAD_DEFINE) + semaphore + startup
    led_a_thread.c        # LED A thread: waits for semaphore, then blinks GPIO A
    led_b_thread.c        # LED B thread: waits for semaphore, then blinks GPIO B
```

---

## Kconfig options

| Option | Default | Description |
|---|---|---|
| `CONFIG_LED_THREAD_STACK_SIZE` | `1024` | Stack size in bytes for each LED thread |
| `CONFIG_LED_A_PRIORITY` | `5` | Thread priority for LED A (lower = higher priority) |
| `CONFIG_LED_B_PRIORITY` | `7` | Thread priority for LED B |
| `CONFIG_LED_A_BLINK_INTERVAL_MS` | `500` | LED A toggle interval in ms |
| `CONFIG_LED_B_BLINK_INTERVAL_MS` | `1000` | LED B toggle interval in ms |
| `CONFIG_LED_A_GPIO_PIN` | `14` | GPIO pin for LED A |
| `CONFIG_LED_B_GPIO_PIN` | `18` | GPIO pin for LED B |
| `CONFIG_LED_SYNC_ENABLED` | `y` | Use semaphore to synchronize thread startup |

---

## Three ways to configure

### 1. Edit `prj.conf` (persistent change)

Open `prj.conf` and change any value. The change is saved for all future builds.

```
# Make LED A blink faster than default
CONFIG_LED_A_BLINK_INTERVAL_MS=200

# Give both threads the same priority to observe round-robin scheduling
CONFIG_LED_A_PRIORITY=5
CONFIG_LED_B_PRIORITY=5

# Disable the semaphore sync — threads start immediately at boot
CONFIG_LED_SYNC_ENABLED=n
```

Then rebuild:

```bash
west build --build-dir examples/multithreads_hello_leds/build
```

### 2. Pass an extra config file at build time (one-off override)

Use `-- -DEXTRA_CONF_FILE=` to layer an additional `.conf` on top of `prj.conf` without modifying it. Useful for experimenting or keeping environment-specific settings separate.

Create e.g. `examples/multithreads_hello_leds/experiment.conf`:

```
# Same priority — forces round-robin scheduling between the two threads
CONFIG_LED_A_PRIORITY=5
CONFIG_LED_B_PRIORITY=5
CONFIG_LED_SYNC_ENABLED=n
```

Then build with it:

```bash
west build -p -b <your_board> examples/multithreads_hello_leds \
    --build-dir examples/multithreads_hello_leds/build \
    -- -DEXTRA_CONF_FILE="experiment.conf"
```

> The `--` separates `west build` arguments from raw CMake arguments. Everything after `--` is passed directly to CMake.

### 3. Interactive menu (`menuconfig`)

Explore **all** available options through a terminal UI:

```bash
# Build first to generate the config, then open the menu
west build -p -b <your_board> examples/multithreads_hello_leds \
    --build-dir examples/multithreads_hello_leds/build

west build --build-dir examples/multithreads_hello_leds/build -t menuconfig
```

Navigate with arrow keys, `Space` to toggle, `Enter` to go into a submenu. Your options appear under **"Multithreaded LEDs Example Configuration"**. Save with `S`, exit with `Q`, then rebuild.

---

## Build

**Replace `-b` with your board identifier.** The code works on any Zephyr-supported board.

```bash
# Build for ESP32 Wrover Kit
west build -p -b esp_wrover_kit/esp32/procpu examples/multithreads_hello_leds \
    --build-dir examples/multithreads_hello_leds/build

# Build for Nordic nRF52840 DK
west build -p -b nrf52840dk/nrf52840 examples/multithreads_hello_leds \
    --build-dir examples/multithreads_hello_leds/build

# Build for STM32 Nucleo F401RE
west build -p -b nucleo_f401re examples/multithreads_hello_leds \
    --build-dir examples/multithreads_hello_leds/build

# Build with an extra config file layered on top of prj.conf
west build -p -b <your_board> examples/multithreads_hello_leds \
    --build-dir examples/multithreads_hello_leds/build \
    -- -DEXTRA_CONF_FILE="experiment.conf"

# Incremental rebuild after a small change
west build --build-dir examples/multithreads_hello_leds/build
```

> **Tip:** Adjust `CONFIG_LED_A_GPIO_PIN` and `CONFIG_LED_B_GPIO_PIN` in `prj.conf` to match the LED pins on your board.

---

## Flash

Always specify `--build-dir` so west knows which binary to flash. West auto-detects the correct flash runner for your board.

```bash
# Generic (works for most boards: Nordic, STM32, etc.)
west flash --build-dir examples/multithreads_hello_leds/build

# ESP32 — use --esp-device to specify the serial port
west flash --build-dir examples/multithreads_hello_leds/build --esp-device /dev/ttyUSB0
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
cd examples/multithreads_hello_leds/build && west espressif monitor -p /dev/ttyUSB0
```

Expected output (the two threads interleave at their own rates):

```
[00:00:00.100,000] <inf> main: === Multithreaded LED Example Started ===
[00:00:00.101,000] <inf> main: LED A: GPIO14, interval 500 ms, priority 5
[00:00:00.102,000] <inf> main: LED B: GPIO18, interval 1000 ms, priority 7
[00:00:00.103,000] <inf> main: Releasing LED threads via semaphore...
[00:00:00.104,000] <inf> led_a: LED A thread running (GPIO14, every 500 ms)
[00:00:00.105,000] <inf> led_b: LED B thread running (GPIO18, every 1000 ms)
[00:00:00.605,000] <inf> led_a: LED A (GPIO14): OFF
[00:00:01.105,000] <inf> led_b: LED B (GPIO18): OFF
[00:00:01.105,000] <inf> led_a: LED A (GPIO14): ON
...
```

---

## Experiments

Try these modifications to deepen your understanding:

| Experiment | How | What to observe |
|---|---|---|
| **Equal priorities** | Set `CONFIG_LED_A_PRIORITY=CONFIG_LED_B_PRIORITY=5` | Round-robin: both threads get equal time slices |
| **No sync** | Set `CONFIG_LED_SYNC_ENABLED=n` | Threads start immediately without the semaphore gate |
| **Tiny stack** | Set `CONFIG_LED_THREAD_STACK_SIZE=256` | Stack overflow — Zephyr crashes or behaves erratically |
| **Same interval** | Set both intervals to `500` | Priority difference becomes visible: LED A always logs before LED B |
| **Slow LED B** | Set `CONFIG_LED_B_BLINK_INTERVAL_MS=5000` | Long sleep on LED B frees CPU time — other threads benefit |
