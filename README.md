# Mister Arp

Arpeggiator synthesizer by **Tully EDM Vibe** — AU, VST3 and standalone app for macOS 12+ (Apple Silicon and Intel).

- 11 arp modes: Up, Down, Up/Down, Down/Up, As Played, Random, Chord, Converge, Diverge, Pinky, Thumb
- 1–4 octaves, 1/32 to 1/4 (dotted and triplet), gate up to 150%, swing, latch
- 16-step pattern: on/off, velocity, ratchet (1–4), chance, per-step transpose, tie; odd lengths for polymeter
- Scale quantiser (11 scales, any key)
- Follows the DAW tempo and song position; free-runs with its own tempo in the standalone app
- DRAG MIDI: drag 1–8 bars of the arp straight onto a DAW track (click to save the .mid)
- Full synth engine (shared with Mister Leads) on the SOUND page
- 48 presets: Trance, Psy, House & Techno, Melodic & Ambient, Synthwave & Retro, Chords & Stabs, Experimental

## Build
    cmake -S . -B build-release -G Xcode -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
    cmake --build build-release --config Release

## Self-test
    cmake -S . -B build-test -DMA_TESTS=ON && cmake --build build-test --config Release --target MATest
Checks every mode's note order, sample-exact timing at several block sizes, swing, ratchets, chance, ties,
scale quantising, latch, host loops, MIDI export, every preset, state save/restore and the editor.

(c)(p) 2026 Tully EDM Vibe — info@tullyedmvibe.com — www.tullyedmvibe.com
