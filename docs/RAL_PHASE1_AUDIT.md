# RAL NeuralNote Lab — Phase 1 Architecture Audit

Status: source review only; build and runtime tests NOT yet performed.
Branch: `ral-development`. Preserve `main` as the upstream baseline.

## Confirmed source architecture

- `NeuralNote/PluginSources/PluginProcessor.cpp`: JUCE audio processor; receives audio and MIDI buffers, owns SourceAudioManager, Player, TranscriptionManager, InstrumentMixer, and persists state.
- `NeuralNote/Source/SourceAudioManager.*`: source-audio handling; inspect before changing import or recording.
- `NeuralNote/Source/TranscriptionManager.*`: transcription lifecycle, background job, pause/resume, progressive note updates, saved state and UI synchronization.
- `Lib/Model/MuscriptorEngine.*`: adapter around `msl::Transcriber`; accepts mono audio at `TRANSCRIPTION_SAMPLE_RATE`, selected instrument groups and model/device options; emits `NoteEvent` values.
- `Lib/Model/NoteEvent.h`: internal note representation; inspect when building chord analysis.
- `NeuralNote/Source/MidiFile/MidiFileWriter.*`: MIDI export path; inspect for MIDI tick/time conversion and per-instrument handling.
- `NeuralNote/Source/InstrumentSelection.*`: user-selected instruments; model conditioning occurs in the engine rather than a simple post-filter.
- `Lib/Player/*` and `NeuralNote/Source/SynthController.*`: MIDI note scheduling and preview synthesis.
- `NeuralNote/Source/UI/*`: existing interface; do not redesign in Phase 1.

## Pipeline (source-backed, high level)

Input/recording → SourceAudioManager → TranscriptionManager background job → MuscriptorEngine → muscriptor.cpp submodule + GGUF weights → NoteEvent vector → playback / piano roll / MIDI export.

The model is loaded per transcription and released afterward. Partial notes can be displayed during decoding; the final result is authoritative. Transcription must not run on the real-time audio or message thread.

## Build and dependencies

- `CMakeLists.txt` identifies project version 2.1.0 and requires C++23, CMake >=3.24.
- `.gitmodules` declares `ThirdParty/JUCE` and `ThirdParty/muscriptor.cpp`; clone with `--recurse-submodules`.
- Python 3 and network access are needed on initial configure for soundfont preparation and dependencies.
- Windows Vulkan SDK is needed for GPU support at build time; CPU-only build is possible without it.
- No compile, binary verification, unit-test run, or audio-to-MIDI quality measurement has been performed in this audit.

## Licensing checkpoints

NeuralNote code: Apache-2.0. muscriptor.cpp: MIT. MuScriptor model weights: CC BY-NC 4.0 (noncommercial). JUCE and other third-party licenses require separate compliance review. Do not commit downloaded GGUF model weights or assume commercial permission.

## Proposed first experiment: read-only MIDI chord analyzer

1. Inspect `NoteEvent.h`, `TranscriptionManager.cpp`, `MidiFileWriter.cpp` and existing tests.
2. Define a pure chord-analysis function consuming copied note events; do not modify model inference.
3. Group simultaneous notes with a configurable onset tolerance, derive pitch classes, recognize basic triads and seventh chords, report ambiguous interpretations rather than inventing certainty.
4. Write deterministic unit tests for major/minor/diminished/augmented triads, dominant/major/minor sevenths, inversions, rests, overlaps and ambiguous voicings.
5. Only after passing tests, propose a UI integration behind an opt-in feature flag.

## Guardrails

- No edits to `main`, no automatic merge, no changes to upstream copyright/license notices.
- Preserve original NeuralNote identifiers and plugin format IDs until a separately tested renamed build exists; changing them can break DAW session compatibility.
- Never perform inference or blocking filesystem operations in `processBlock`.
- Benchmark before making speed or accuracy claims.
- Keep audio files, downloaded models and build outputs out of Git.
