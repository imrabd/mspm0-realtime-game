# MSPM0 Real-Time Game Engine

A complete real-time game running bare-metal on an MSPM0G3507 (ARM Cortex-M0+,
80 MHz) with no RTOS. Two independent real-time loops share one core: game
logic at 30 Hz and audio synthesis at 11,025 Hz.

A soccer-themed arcade game on a 128x160 ST7735 LCD, with a four-language
menu system, PCM sound effects streamed from flash through a DAC, and
slide-pot plus switch input.

## Engineering notes

**Two real-time loops on one core.** A `TIMG12` periodic interrupt drives game
physics, 12-bit ADC sampling of the slide pot, and switch debouncing at 30 Hz.
A separate `SysTick` interrupt runs at 11,025 Hz pushing PCM samples to the
DAC. The audio loop can never miss its deadline without an audible artifact,
so it holds the higher priority and does the minimum work per tick.

**Keeping slow SPI writes out of interrupt context.** Writing a frame to the
ST7735 over SPI takes far longer than one audio period, so display updates
happen in the main loop rather than an ISR. A frame semaphore signals when a
redraw is due and `volatile` mailboxes carry state out of the interrupt
handlers, so audio timing holds regardless of how long a frame takes.

**Interrupt-safe serial input.** A circular FIFO with receive timeouts limits
the UART ISR to byte capture while the main loop parses framed messages.
Bytes survive even when the renderer is busy.

**Four-language menus.** English, Spanish, Portuguese, and French, using an
extended character set for accented glyphs.

## Layout

| Path | Contents |
|---|---|
| `game_engine_main.c` | Game state machine, physics, rendering, interrupt handlers |
| `Sound.c` / `Sound.h` | DAC audio driver, SysTick sample streaming |
| `Switch.c` | Debounced switch input |
| `sprites/SUISprites.h` | Sprite and background art, RGB565 |
| `SUISounds/` | Sound effects, `.wav` sources and converted tables |

## Building

Targets the MSPM0G3507 LaunchPad, built with TI Code Composer Studio.

Depends on the MSPM0 peripheral driver set — `ST7735`, `Clock`, `LaunchPad`,
`Timer`, `ADC1`, `DAC`, `TExaS`, `Arabic`, plus `SmallFont`, `UART`,
`Switch.h` and `LED` support files — which are **not redistributed here**.
Supply them from an MSPM0 ValvanoWare distribution, along with the CCS project
files and the MSPM0G3507 linker command file, then drop these sources in
alongside.

Note that `game_engine_main.c` retains several standalone demo entry points
(`main0` through `main4`) from the original project scaffolding; the game
itself is `main`.

## Attribution

The game design, engine architecture, interrupt structure, audio driver,
sprite art, and sound assets here are my own work. The underlying peripheral
drivers and project scaffolding are by Jonathan Valvano and are not included
in this repository.
