# u585-verification-framework

A ThreadX/NetXDuo-based firmware project for the STM32U585 (B-U585I-IOT02A
board): reads the board's on-board sensor suite, captures JPEG frames from
an OV5640 camera, and serves both over Wi-Fi via HTTP.

## Features

- **Sensor suite** (I2C2): ambient light (VEML6030), humidity/temperature
  (HTS221), pressure/temperature (LPS22HH), magnetometer (IIS2MDC), and
  IMU (ISM330DHCX). Polled periodically by `Sensor_Service` and exposed
  through simple getter functions.
- **Camera** (I2C1 + DCMI/DMA): OV5640 JPEG capture, double-buffered so a
  new frame can be captured while the previous one is served.
- **Wi-Fi**: EMW3080 module (`MX_WIFI` driver stack) over NetXDuo's TCP/IP
  stack, with DHCP.
- **HTTP server** (NetXDuo Web HTTP addon), served as a small dashboard:
  - `/` - home page, links to the camera and sensor pages
  - `/camera.html` - live camera view (embeds `/stream.jpg`)
  - `/sensors.html` - live sensor readings (polls `/sensors.json` once a second)
  - `/image.jpg` - the most recently captured frame
  - `/stream.jpg` - MJPEG (`multipart/x-mixed-replace`) live stream, served
    on its own port so a long-lived stream connection never blocks the
    other routes
  - `/sensors.json` - a snapshot of the current sensor readings
  - `/hello` - liveness check

## Screenshots

The dashboard running live on a B-U585I-IOT02A, served entirely from the
board itself over Wi-Fi:

| `/camera.html` | `/sensors.html` |
| --- | --- |
| ![Live camera view](docs/images/camera-page.png) | ![Live sensor readings](docs/images/sensors-page.png) |

## Architecture

```mermaid
flowchart TB
    subgraph HW["On-board hardware"]
        I2C2[I2C2 bus]
        I2C1[I2C1 bus]
        WIFIHW[EMW3080 Wi-Fi module]
    end

    subgraph PLATFORM["Platform/ - hardware-agnostic drivers"]
        MEMS["VEML6030 / HTS221 / LPS22HH /\nIIS2MDC / ISM330DHCX"]
        OV5640[OV5640 camera]
        MXWIFI[MX_WIFI driver stack]
    end

    subgraph APP["Application/ - services"]
        I2CBUS["I2C_Bus (mutex-serialized)"]
        SENSORSVC[Sensor_Service]
        CAMSVC[Camera_Service]
        WIFIBUS[WiFi_Bus]
        NETSVC["Network_Service + http_responses"]
    end

    SYS[["System.c - composition root"]]

    subgraph RTOS["ThreadX / NetXDuo"]
        TX[ThreadX threads and mutexes]
        NX["NetXDuo TCP/IP + Web HTTP server"]
    end

    CLIENT([Browser])

    I2C2 --> MEMS
    I2C1 --> OV5640
    WIFIHW --> MXWIFI

    MEMS --> I2CBUS
    I2CBUS --> SENSORSVC
    OV5640 --> CAMSVC
    MXWIFI --> WIFIBUS
    WIFIBUS --> NETSVC

    SYS --> SENSORSVC
    SYS --> CAMSVC
    SYS -.->|registers providers| NETSVC
    SENSORSVC -.->|"sensors.json provider"| NETSVC
    CAMSVC -.->|"image.jpg / stream.jpg provider"| NETSVC

    SENSORSVC --> TX
    CAMSVC --> TX
    NETSVC --> NX
    NX --> TX

    NETSVC -->|"/, /camera.html, /sensors.html,\n/image.jpg, /stream.jpg, /sensors.json"| CLIENT
```

`Network_Service` never includes `camera_service.h`/`sensor_service.h` directly - `System.c`
registers a plain function pointer for each (`Http_Image_Provider_t` /
`Http_Text_Provider_t`, `Application/Network/http_responses.h`), so the
dotted arrows above are the only coupling between them.

- **`Platform/`** - hardware-agnostic component drivers (mostly vendor
  sources), each taking its I/O (register read/write, delay, tick) as
  injected function pointers rather than calling the HAL directly.
- **`Application/`** - board-specific glue and business logic. Each
  feature is its own module following an `Init()` / `Start()` shape:
  `Init()` brings the driver up, `Start()` creates its ThreadX thread(s).
  Shared peripherals (e.g. an I2C bus used by more than one sensor) are
  passed in explicitly rather than reached for via global state.
- **`Application/System/system.c`** - the composition root: the one place
  that wires concrete modules together (which sensor uses which I2C bus,
  which function feeds the camera's HTTP responses, and so on).
- CubeMX-generated glue (`Core/`, `NetXDuo/App/`, `AZURE_RTOS/App/`) is
  kept intentionally thin - real logic lives under `Application/`, so
  regenerating the CubeMX project doesn't overwrite it.

## Building

The firmware cross-compiles with `arm-none-eabi-gcc` via CMake + Ninja:

```sh
cmake --preset Debug
cmake --build build/Debug
```

(A `Release` preset is also available.) The project can also be opened
directly in STM32CubeIDE.

## Tests

Two host-native test suites build and run on the PC, independent of the
firmware's cross-compiled build:

- **`Tests/Unit`** - one driver's logic tested in isolation, with its
  register-level dependencies mocked (via CMock).
- **`Tests/Integration`** - the seams between this project's own modules
  and ThreadX/NetXDuo, some tested against the real vendor RTOS source
  rather than mocks.

```sh
cmake -S Tests/Unit -B Tests/Unit/build
cmake --build Tests/Unit/build
ctest --test-dir Tests/Unit/build --output-on-failure
```

(Same pattern for `Tests/Integration`.)

## License

The original source code developed for this project is licensed under
the MIT License.

Third-party components, including STM32 software components provided
by STMicroelectronics, remain under their respective licenses.
See the corresponding license files and package license information.
