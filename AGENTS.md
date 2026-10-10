# Repository Guidelines

## Project Structure

This repository is a WIP 1:1 decompilation of Mario Kart: Double Dash!!. Game and system source is under `src/`, public declarations under `include/`, and third-party/runtime code under `libs/`. Version-specific build metadata, symbols, splits, and hashes are in `config/MarioClub_us/` and `config/Release_eu/`. Original inputs and extracted assets belong under `orig/`; generated objects and binaries are under `build/`. Development helpers are in `tools/`.

## Build and Development Commands

- `python3 configure.py` — configure the default USA debug build and refresh Ninja files.
- `python3 configure.py progress` — show matching and linking progress without rebuilding.
- `ninja` — build the configured project.
- `ninja build/MarioClub_us/src/Kameda/Race2D.o` — rebuild one object while iterating on a source file.
- `build/tools/objdiff-cli report generate` — generate a matching progress report.
- `build/tools/objdiff-cli diff -p . -u main/Kameda/Race2D <symbol>` — inspect a function-level comparison; use an interactive terminal.

The GameCube compiler runs through `wibo` or Wine. In restricted environments, compiler and objdiff commands may need elevated execution permissions.

## Coding Style and Naming

Use four-space indentation, braces on the same line, and the existing C/C++ naming conventions. Preserve established namespaces and subsystem ownership (for example, `src/Kameda/` with `include/Kameda/`). Match original function signatures, address comments, data types, and ordering where possible. Avoid broad formatting changes; generated files and build output should not be committed.

## Testing and Verification

There is no conventional unit-test suite. Validation is binary matching: rebuild the affected object, run objdiff, and inspect the relevant function or symbol. Use `git diff --check` before committing. Keep changes small enough that a mismatch can be traced to one function or data definition.

## Commits and Pull Requests

Recent commits use concise imperative descriptions such as `Match and link ...`. Follow that pattern and identify the matched source unit or subsystem. Pull requests should explain the affected object/functions, include matching or progress results, and mention any build-environment requirements. Do not include game assets or assembly; the repository expects an existing game copy locally.
