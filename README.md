# Chess-Engine-C
A chess engine written in C

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
