# Chess-Engine-C
A chess engine written in C

## Project layout

The source tree is now organized as follows:

- `src/app/` - application entrypoint, command processing, tests
- `src/engine/` - engine logic and core chess model code
- `src/util/` - utility helpers and support libraries
- `src/logging/` - logging implementation and config
- `src/ui/` - pluggable UI backends and common UI helpers

## Build

The project uses a simple `Makefile` with a selectable UI backend.

Default build (terminal UI):

    make

Select a different UI backend with `UI=`:

    make UI=none
    make UI=raylib

The UI code is now organized under `src/ui/`, with a common UI header and backend stubs for:

- `terminal` (default)
- `none`
- `raylib`
