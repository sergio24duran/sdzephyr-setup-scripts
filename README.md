# SDZephyr Setup Scripts

A one-command setup for [Zephyr RTOS](https://zephyrproject.org/) projects, bundled with educational examples that teach you the fundamentals of real-time operating systems.

---

## Table of Contents

- [What is an RTOS?](#what-is-an-rtos)
- [Why Zephyr?](#why-zephyr)
- [Key Zephyr Concepts](#key-zephyr-concepts)
- [Board Support](#board-support)
- [Prerequisites](#prerequisites)
- [Quick Start](#quick-start)
- [Setup Script Reference](#setup-script-reference)
- [Examples](#examples)
- [Building and Flashing](#building-and-flashing)
- [Useful Commands](#useful-commands)
- [Project Structure](#project-structure)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

---

## What is an RTOS?

A **Real-Time Operating System (RTOS)** is an operating system designed to process data and events within strict time constraints. Unlike general-purpose operating systems (Linux, Windows), an RTOS provides **deterministic** behavior — you can predict and guarantee how quickly the system responds to events.

### Key characteristics

| Feature | General-Purpose OS | RTOS |
|---|---|---|
| **Scheduling** | Optimized for throughput / fairness | Optimized for timing guarantees |
| **Latency** | Variable (milliseconds to seconds) | Predictable (microseconds) |
| **Preemption** | May delay high-priority tasks | Higher-priority tasks run immediately |
| **Memory** | Requires MB/GB of RAM | Runs in KB of RAM |
| **Use case** | Desktops, servers | Embedded devices, IoT, robotics |

### Core RTOS concepts

1. **Threads** — Independent units of execution, each with its own stack and priority. In Zephyr, threads can be created statically at compile time (`K_THREAD_DEFINE`) or dynamically at runtime (`k_thread_create`).

2. **Scheduler** — The kernel component that decides which thread runs next. Zephyr uses a **priority-based preemptive scheduler**: the highest-priority ready thread always runs. Threads of equal priority share the CPU in round-robin fashion.

3. **Priorities** — Each thread has a priority. In Zephyr, **lower number = higher priority**. Priority 0 is the highest preemptive priority. Negative priorities create **cooperative threads** that must explicitly yield.

4. **Synchronization primitives** — Mechanisms for threads to coordinate:
   - **Semaphores** (`k_sem`) — Signal between threads ("data is ready", "start now")
   - **Mutexes** (`k_mutex`) — Protect shared resources (only one thread at a time)
   - **Message queues** (`k_msgq`) — Pass data safely between threads
   - **Events** (`k_event`) — Signal one or more conditions simultaneously

5. **Kernel timing** — `k_msleep()` puts a thread to sleep without burning CPU cycles. The kernel wakes it after the specified time, allowing other threads to run in the meantime.

---

## Why Zephyr?

[Zephyr RTOS](https://zephyrproject.org/) is an open-source, scalable real-time operating system optimized for resource-constrained devices. It is a Linux Foundation project with backing from Intel, Nordic, NXP, and many others.

### Highlights

- **500+ board support** — ESP32, nRF52, STM32, Arduino, Raspberry Pi Pico, and many more
- **Small footprint** — Kernel can fit in as little as 8 KB of RAM
- **Modular architecture** — Enable only what you need via Kconfig
- **Built-in drivers** — GPIO, UART, SPI, I2C, BLE, Wi-Fi, USB, and more
- **Security** — TLS/DTLS, secure boot, and crypto APIs built in
- **Modern build system** — CMake + West + Devicetree + Kconfig

---

## Key Zephyr Concepts

### West

**West** is Zephyr's command-line meta-tool. It manages:
- Workspace initialization (`west init`)
- Module/dependency management (`west update`)
- Building (`west build`)
- Flashing firmware (`west flash`)
- Debugging (`west debug`)

### Kconfig

**Kconfig** (inherited from the Linux kernel) is the build-time configuration system. Enable/disable features and set parameters without changing source code.

- Options are defined in `Kconfig` files
- Values are set in `prj.conf` or extra `.conf` files
- Each option gets the `CONFIG_` prefix in code and config files
- Explore all options interactively: `west build -t menuconfig`

> **Important:** If your application provides its own `Kconfig` file, it becomes the root of the Kconfig tree and **must** start with `source "Kconfig.zephyr"` to pull in all Zephyr symbols (`CONFIG_GPIO`, `CONFIG_LOG`, etc.). Without this line, those symbols are undefined and the build fails.

### Devicetree

Describes the hardware layout (peripherals, addresses, pin assignments). Zephyr reads it at build time to generate C macros for hardware access. Override board defaults with `.overlay` files.

### CMakeLists.txt

Each application has a `CMakeLists.txt` that loads the Zephyr package (`find_package(Zephyr)`) and lists source files.

---

## Board Support

**This project is board-agnostic.** All example code uses standard Zephyr APIs (GPIO, kernel, logging) that work on any of the 500+ boards Zephyr supports. No source code changes are needed to switch boards — just change the `-b <board>` flag in your build command.

### How to find your board identifier

```bash
# List all supported boards
west boards

# Filter by vendor/family
west boards | grep esp32         # Espressif ESP32
west boards | grep nrf           # Nordic nRF52/nRF53/nRF91
west boards | grep nucleo        # ST Nucleo (STM32)
west boards | grep arduino       # Arduino boards
west boards | grep rpi_pico      # Raspberry Pi Pico
```

### Common board identifiers

| Board | Identifier |
|---|---|
| ESP32 Wrover Kit | `esp_wrover_kit/esp32/procpu` |
| ESP32-DevKitC | `esp32_devkitc_wroom` |
| nRF52840 DK | `nrf52840dk/nrf52840` |
| STM32 Nucleo-H755ZI | `nucleo_h755zi_q/stm32h755xx/m7` |
| Arduino Nano 33 BLE | `arduino_nano_33_ble` |
| Raspberry Pi Pico | `rpi_pico` |

### What you may need to change per board

| Setting | Why | How |
|---|---|---|
| **Board identifier** (`-b`) | Different hardware target | Change the `-b` flag in `west build` |
| **GPIO pin numbers** | LED pins differ between boards | Edit `prj.conf` (e.g. `CONFIG_HELLO_LED_GPIO_PIN=13`) |
| **Toolchain** | Each architecture needs its own compiler | Install the [Zephyr SDK](https://docs.zephyrproject.org/latest/develop/toolchains/zephyr_sdk.html) (covers most boards) or vendor-specific toolchain |
| **Flash runner** | Flashing mechanism varies by board | `west flash` auto-detects the correct runner for your board |
| **Serial monitor** | Port and tool may differ | Use `minicom`, `screen`, `picocom`, or vendor-specific tools |

> **Note:** The examples in this README use ESP32 as the primary target because it is one of the most popular development boards. Replace the board identifier and GPIO pins to use any other supported board.

---

## Prerequisites

- **OS**: Ubuntu 20.04+ / Debian 11+ (other Linux distros work with equivalent packages)
- **Disk space**: ~5 GB (Zephyr source + modules + toolchains)
- **Internet**: Required during setup

The setup script checks and optionally installs all required packages automatically.

### Required host tools

| Tool | Purpose |
|---|---|
| `cmake` | Build system generator |
| `ninja` | Fast build executor |
| `python3` + `pip3` | Zephyr scripts and West |
| `dtc` | Devicetree compiler |
| `gperf` | Perfect hash generator |
| `ccache` | Compilation cache |
| `west` | Zephyr meta-tool (installed via pip) |
| `git` | Version control |
| `gcc` / `g++` | Host compiler |
| `wget` | Downloading files |

---

## Quick Start

```bash
# 1. Clone this repository
git clone https://gitlab.com/sdzephyr/sdzephyr-setup-scripts.git
cd sdzephyr-setup-scripts

# 2. Make the setup script executable
chmod +x zephyr-setup.sh

# 3. Set up a new workspace (auto-install deps, include examples)
./zephyr-setup.sh -p /home/$USER/my-zephyr-project -y -e

# 4. Enter the workspace and load the Zephyr environment
cd /home/$USER/my-zephyr-project
source zephyr/zephyr-env.sh

# 5. Build the Hello LED example (replace -b with YOUR board)
west build -p -b esp_wrover_kit/esp32/procpu examples/hello_led \
    --build-dir examples/hello_led/build

# 6. Flash to the board
west flash --build-dir examples/hello_led/build

# 7. Monitor serial output (method depends on your board — see below)
minicom -D /dev/ttyUSB0 -b 115200
```

> **Using a different board?** Just replace `esp_wrover_kit/esp32/procpu` with your board identifier (see [Board Support](#board-support)). You may also need to adjust the GPIO pin in `prj.conf` to match your hardware.

---

## Setup Script Reference

### Usage

```bash
./zephyr-setup.sh -p <absolute-path> [-d] [-y] [-e] [-h]
```

### Options

| Flag | Description |
|---|---|
| `-p PATH` | **(Required)** Absolute path where the Zephyr project will be created |
| `-d` | Skip dependency check entirely — manage your own dependencies |
| `-y` | Auto-install missing dependencies without prompting (invalid with `-d`) |
| `-e` | Copy bundled example projects into the workspace |
| `-h` | Show help message |

> **Note:** `-d` and `-y` are mutually exclusive. Using both together is an error.

### What the script does

1. **(Optional) Dependency check** — Scans for required tools and offers to install them. Skipped with `-d`.
2. **West init** — Creates a new Zephyr workspace (skipped if one already exists).
3. **West update** — Downloads all Zephyr modules (HALs, libraries, etc.).
4. **Python deps** — Installs Zephyr's Python requirements.
5. **.gitignore** — Always copies a pre-configured `.gitignore` for Zephyr workspaces.
6. **(Optional) Examples** — Copies the bundled example projects. Enabled with `-e`.

### Usage examples

```bash
# Minimal: skip dependency check, no examples (clean workspace only)
./zephyr-setup.sh -p /home/user/zephyr-workspace -d

# Check/install dependencies interactively, no examples
./zephyr-setup.sh -p /home/user/zephyr-workspace

# Auto-install dependencies and include examples
./zephyr-setup.sh -p /home/user/zephyr-workspace -y -e

# Skip dependency check but include examples (useful if you already have dependencies set up)
./zephyr-setup.sh -p /home/user/zephyr-workspace -d -e
```

---

## Examples

Each example has its own README with full build/flash instructions, Kconfig reference, and suggested experiments.

| Example | Description | README |
|---|---|---|
| **hello_led** | Blinks an LED — GPIO driver, kernel sleep, logging, and Kconfig | [examples/hello_led/README.md](zephyr-resources/examples/hello_led/README.md) |
| **multithreads_hello_leds** | Two threads blink two LEDs — threads, priorities, and semaphores | [examples/multithreads_hello_leds/README.md](zephyr-resources/examples/multithreads_hello_leds/README.md) |

---

## Building and Flashing

### The `west build` command

```
west build [-p] -b <board> <app-path> --build-dir <build-dir> [-- -DCMAKE_VAR=value]
```

| Argument | Description |
|---|---|
| `-p` | Pristine (clean) build. Always use when switching boards or after major changes. |
| `-b <board>` | Target board identifier (e.g. `esp_wrover_kit/esp32/procpu`, `nrf52840dk/nrf52840`) |
| `<app-path>` | Path to the application directory |
| `--build-dir <dir>` | Where to put build artifacts. Keep them inside the app folder to stay organized. |
| `-- -DVAR=value` | Pass CMake variables directly. `--` separates west args from CMake args. |

### The `west flash` command

```
west flash --build-dir <build-dir> [--runner <runner>] [runner-specific-options]
```

Always specify `--build-dir` so west knows which binary to flash. West automatically selects the correct flash runner for your board (e.g. `esptool` for ESP32, `nrfjprog` for Nordic, `openocd` for STM32).

**ESP32 boards** — use `--esp-device` to specify the serial port:
```bash
west flash --build-dir examples/hello_led/build --esp-device /dev/ttyUSB0
```

**Most other boards** — `west flash` auto-detects the connected device:
```bash
west flash --build-dir examples/hello_led/build
```

---

### Three ways to configure a build

Zephyr offers three complementary approaches. They can be combined.

#### 1. Edit `prj.conf` — permanent defaults

The simplest approach. Edit `prj.conf` in your app directory and rebuild. Changes persist across all future builds of that app.

```bash
# Edit prj.conf, then rebuild incrementally (no -p needed for config-only changes)
west build --build-dir examples/hello_led/build
```

#### 2. Extra config file — one-off or layered overrides

Pass an additional `.conf` file with `-- -DEXTRA_CONF_FILE=`. It merges **on top of** `prj.conf` — only the listed values are overridden, the original file is untouched.

Ideal for:
- Trying a different configuration without touching `prj.conf`
- Keeping sensitive data (credentials, API keys) in a gitignored file

```bash
# Build hello_led with a fast-blink override (replace -b with your board)
west build -p -b <your_board> examples/hello_led \
    --build-dir examples/hello_led/build \
    -- -DEXTRA_CONF_FILE="fast_blink.conf"

# Real-world example: a networked app with credentials kept out of git
west build -p -b <your_board> examples/my_app \
    --build-dir examples/my_app/build \
    -- -DEXTRA_CONF_FILE="credentials.conf"
```

> Pass multiple files by separating them with semicolons:
> `-DEXTRA_CONF_FILE="fileA.conf;fileB.conf"`

#### 3. Interactive `menuconfig` — explore everything

A full terminal UI to browse and change any Kconfig option in the system.

```bash
# Open menuconfig for an existing build directory
west build --build-dir examples/hello_led/build -t menuconfig
```

- Arrow keys to navigate, `Space` to toggle, `Enter` for submenus
- Your app options appear under their named menu group
- Save with `S`, quit with `Q`, then rebuild:

```bash
west build --build-dir examples/hello_led/build
```

---

### Complete build & flash examples

> The examples below use ESP32 as the target. **Replace the `-b` flag with your board identifier** — the code works on any Zephyr-supported board.

#### ESP32

```bash
# ── hello_led ──────────────────────────────────────────────────────────────

# Clean build
west build -p -b esp_wrover_kit/esp32/procpu examples/hello_led \
    --build-dir examples/hello_led/build

# With an extra conf file
west build -p -b esp_wrover_kit/esp32/procpu examples/hello_led \
    --build-dir examples/hello_led/build \
    -- -DEXTRA_CONF_FILE="fast_blink.conf"

# Flash (--esp-device specifies the serial port)
west flash --build-dir examples/hello_led/build --esp-device /dev/ttyUSB0

# Monitor (requires esptool: pip install esptool)
cd examples/hello_led/build && west espressif monitor -p /dev/ttyUSB0


# ── multithreads_hello_leds ─────────────────────────────────────────────────

# Clean build
west build -p -b esp_wrover_kit/esp32/procpu examples/multithreads_hello_leds \
    --build-dir examples/multithreads_hello_leds/build

# With an extra conf file
west build -p -b esp_wrover_kit/esp32/procpu examples/multithreads_hello_leds \
    --build-dir examples/multithreads_hello_leds/build \
    -- -DEXTRA_CONF_FILE="experiment.conf"

# Flash
west flash --build-dir examples/multithreads_hello_leds/build --esp-device /dev/ttyUSB0

# Monitor
cd examples/multithreads_hello_leds/build && west espressif monitor -p /dev/ttyUSB0
```

#### Nordic nRF52840 DK

```bash
# Build hello_led (adjust GPIO pin in prj.conf if needed: CONFIG_HELLO_LED_GPIO_PIN=13)
west build -p -b nrf52840dk/nrf52840 examples/hello_led \
    --build-dir examples/hello_led/build

# Flash (auto-detects connected J-Link)
west flash --build-dir examples/hello_led/build

# Monitor serial output
minicom -D /dev/ttyACM0 -b 115200
```

#### STM32 Nucleo

```bash
# Build hello_led (adjust GPIO pin in prj.conf to match your board's LED)
west build -p -b nucleo_f401re examples/hello_led \
    --build-dir examples/hello_led/build

# Flash (auto-detects connected ST-Link)
west flash --build-dir examples/hello_led/build

# Monitor serial output
minicom -D /dev/ttyACM0 -b 115200
```

#### Any board — incremental rebuild

```bash
# After the initial build, incremental rebuilds don't need -p or -b
west build --build-dir examples/hello_led/build
```

> Find your serial port: `ls /dev/ttyUSB* /dev/ttyACM*`

---

## Useful Commands

```bash
# List supported boards
west boards

# Search for boards by vendor/family
west boards | grep esp32          # Espressif
west boards | grep nrf            # Nordic
west boards | grep nucleo         # ST Nucleo (STM32)
west boards | grep arduino        # Arduino

# Open interactive Kconfig menu
west build --build-dir <build-dir> -t menuconfig

# Incremental rebuild
west build --build-dir <build-dir>

# Clean build directory
west build --build-dir <build-dir> -t clean

# Debug with GDB
west debug --build-dir <build-dir>

# Workspace info
west topdir
west list
```

---

## Project Structure

After running the setup script:

```
my-zephyr-project/
  .west/                       # West workspace metadata
  .gitignore                   # Pre-configured for Zephyr
  bootloader/                  # MCUboot (downloaded by west)
  modules/                     # Zephyr modules: HALs, crypto, etc.
  tools/                       # Build tools
  zephyr/                      # Zephyr RTOS source
    zephyr-env.sh              # Load once per shell session before building
    boards/                    # Board definitions
    drivers/                   # Device drivers
    kernel/                    # Kernel source
  examples/                    # Bundled examples (requires -e during setup)
    hello_led/                 # Example 1: blinky
    multithreads_hello_leds/   # Example 2: multithreaded LEDs
```

---

## Troubleshooting

### `west espressif monitor` — `No module named 'esptool'`

`west espressif monitor` requires `esptool` in the **active Python environment**. Install it and make sure to use `-p` for the port (not `--esp-device`):

```bash
pip install esptool

# Run from inside the build directory
cd examples/hello_led/build && west espressif monitor -p /dev/ttyUSB0
```

If you are using conda or a virtual environment, make sure `esptool` is installed inside it.

### `west: command not found`

```bash
export PATH="$HOME/.local/bin:$PATH"
# Add to ~/.bashrc to make permanent
```

### `CMake Error: Zephyr package not found`

```bash
source zephyr/zephyr-env.sh
```

### `west update` is very slow or fails midway

The first run downloads several GB. Just re-run to resume:

```bash
west update
```

### Build fails: `toolchain not found`

For ESP32:
```bash
west espressif install
```

For other boards, install the [Zephyr SDK](https://docs.zephyrproject.org/latest/develop/toolchains/zephyr_sdk.html).

### `Permission denied` when flashing

```bash
sudo usermod -aG dialout $USER
# Log out and back in
```

### Kconfig error: `attempt to assign value to undefined symbol`

Your app's `Kconfig` file is being used as the Kconfig root but is missing Zephyr's tree. Add these two lines at the very top of your `Kconfig`:

```kconfig
mainmenu "My Application"
source "Kconfig.zephyr"
```

---

## Contributing

### Repository structure

This project lives in two places:

| Platform | URL | Role |
|---|---|---|
| **GitLab** | `gitlab.com/sdzephyr/sdzephyr-setup-scripts` | Primary — development, Merge Requests, CI/CD |
| **GitHub** | `github.com/sdzephyr/sdzephyr-setup-scripts` | Public mirror — read-only, for discoverability |

**GitLab is the source of truth.** Every merge into the `develop` branch automatically triggers a CI job that pushes the latest state to GitHub. The GitHub repository is kept in sync but is not the place to contribute.

### How to contribute

1. Fork the repository **on GitLab**
2. Create a feature branch: `git checkout -b feature/my-improvement`
3. Commit your changes
4. Push and open a **Merge Request on GitLab**

### Reporting issues

- **Bugs or feature requests:** open an issue on GitLab or GitHub — both are monitored.
- **Fixes** will be developed on GitLab and automatically mirrored to GitHub after merging.

---

## License

This project is open source. See the repository for license details.
