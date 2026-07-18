# godot-rnnoise

A Godot 4 GDExtension that exposes [RNNoise](https://github.com/xiph/rnnoise)
(a recurrent-neural-network noise suppressor) directly to GDScript, as both:

- **`AudioEffectRNNoise`** — drop it onto an Audio Bus in the editor to denoise
  live audio (e.g. microphone input) with no script code.
- **`RNNoiseDenoiser`** — a `RefCounted` class for scripted, frame-by-frame
  denoising of any mono float buffer (files, network audio, etc.).

RNNoise operates on fixed 480-sample (10ms) mono frames at a 48kHz sample rate.
Both APIs enforce this: `AudioEffectRNNoise` requires the project's audio mix
rate to be 48kHz (it warns at runtime if not), and `RNNoiseDenoiser.process_frame()`
requires buffers of exactly `get_frame_size()` samples.

## Building

Requires [SCons](https://scons.org/), [CMake](https://cmake.org/), and a
C/C++ toolchain (on Linux/macOS, also [Ninja](https://ninja-build.org/)).

```sh
git clone --recurse-submodules <this-repo-url>
cd godot-rnnoise
scons platform=<linux|windows|macos> target=template_debug
scons platform=<linux|windows|macos> target=template_release
```

If you cloned without `--recurse-submodules`, run
`git submodule update --init --recursive` first.

Built libraries are written to `example/addons/godot_rnnoise/bin/`. That
`addons/godot_rnnoise/` folder is a self-contained Godot addon — copy it into
any Godot 4.3+ project's `addons/` directory to use the extension there.

## Example project

`example/` is a runnable Godot project demonstrating both APIs:

- `mic_bus_demo.tscn` — live microphone denoising via `AudioEffectRNNoise` on
  a bus, with a checkbox to A/B compare the effect on/off. Open it in the
  Godot 4.3 editor and run it (needs a working microphone).
- `scripted_demo.gd` — a headless demo of `RNNoiseDenoiser`: synthesizes a
  noisy tone, denoises it frame-by-frame, and prints the RMS noise level
  before/after. Run it with:

  ```sh
  godot --headless --path example --script res://scripted_demo.gd
  ```

The first time you open `example/` (in the editor or headless), Godot needs
one pass to discover and cache the GDExtension before classes like
`RNNoiseDenoiser` become available to scripts — if you hit "Cannot get class"
errors on a fresh checkout, open the project in the editor once first.

## Licensing

This extension's own code is BSD-licensed (matching RNNoise). RNNoise is
vendored via the [`werman/noise-suppression-for-voice`](https://github.com/werman/noise-suppression-for-voice)
submodule, used only for the `external/rnnoise/` subdirectory it contains
(vendored [xiph/rnnoise](https://github.com/xiph/rnnoise) plus a portable
CMake build — nothing from that repo's own GPLv3-licensed wrapper code is
built or linked). Note, though, that the submodule's repository root carries
a GPLv3 `LICENSE` file covering the wrapper project as a whole; check that
this fits your use case before distributing this extension.
