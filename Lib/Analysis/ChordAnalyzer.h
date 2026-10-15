//
// RAL NeuralNote Lab — Phase 2A
// Chord analysis engine.
//
// A standalone, dependency-light module that reads NeuralNote's existing NoteEvent data and
// reports the chords it finds. It does not touch the transcription engine, the audio callbacks or
// any JUCE UI type: the only project header it needs is NoteEvent.h, and it reads a copy of the
// note list rather than mutating it, so it is safe to run on the message thread or a worker.
//
// Design goals (so this can later move into an independent plugin unchanged):
//   - No JUCE dependency, no transcription-engine dependency.
//   - Pure functions over immutable input; deterministic output for a given input.
//   - Never invent a chord: when the sounding notes do not match a known template, the segment is
//     reported with no candidate rather than a forced name.
//   - Velocity is ignored on purpose: the transcription pipeline writes a fixed synthetic velocity
//     (see FIXED_NOTE_AMPLITUDE), so it carries no dynamic information to analyse.
//

#ifndef RAL_ChordAnalyzer_h
#define RAL_ChordAnalyzer_h

#include <string>
#include <vector>

#include "NoteEvent.h"

namespace RAL::Analysis
{

/** The chord qualities the engine can name. */
enum class ChordQuality {
    MajorTriad,
    MinorTriad,
    DiminishedTriad,
    AugmentedTriad,
    DominantSeventh,
    MajorSeventh,
    MinorSeventh,
    HalfDiminishedSeventh, // m7♭5
    FullyDiminishedSeventh // dim7
};

/** Human-readable quality suffix, e.g. "", "m", "dim", "aug", "7", "maj7", "m7", "m7b5", "dim7". */
const char* chordQualitySuffix(ChordQuality inQuality);

/** @return How many chord tones the quality has (3 for a triad, 4 for a seventh). */
int chordQualityToneCount(ChordQuality inQuality);

/**
 * One possible reading of a sounding note set. A segment can carry several of these when the pitch
 * classes are genuinely ambiguous (a symmetric chord such as an augmented triad or a fully
 * diminished seventh maps onto more than one root).
 */
struct ChordCandidate {
    int rootPitchClass = -1; // 0-11
    ChordQuality quality = ChordQuality::MajorTriad;
    int bassPitchClass = -1; // 0-11, the lowest sounding note's pitch class
    int inversion = 0; // 0 root position, 1 first, 2 second, 3 third (sevenths only)
    double confidence = 0.0; // 0-1; 1 for an unambiguous exact match, lower when roots compete
    std::string name; // e.g. "Cmaj7", "C/E" (slashed when not in root position)
};

/**
 * The notes sounding together over one span of time and what they were read as.
 * candidates is ranked best-first and is empty when no reliable interpretation exists.
 */
struct ChordSegment {
    double startTime = 0.0; // seconds; earliest onset in the group
    double endTime = 0.0; // seconds; latest offset in the group
    std::vector<int> pitches; // every sounding MIDI pitch, sorted ascending (octaves kept)
    std::vector<int> pitchClasses; // unique pitch classes present, sorted ascending (octaves folded)
    std::vector<ChordCandidate> candidates; // ranked; empty => no reliable chord
    bool isAmbiguous = false; // true when more than one candidate survived

    /** @return The best reading, or nullptr when the segment names no chord. */
    const ChordCandidate* best() const { return candidates.empty() ? nullptr : &candidates.front(); }
};

/** Tunables. Defaults are deliberately conservative. */
struct ChordAnalyzerOptions {
    // Notes whose onsets fall within this window of the group's first onset are treated as struck
    // together. Seconds, matching NoteEvent's own unit. 0 means exact-onset grouping only.
    double onsetToleranceSeconds = 0.05;

    // Drop groups shorter than this (seconds). 0 keeps everything.
    double minSegmentDurationSeconds = 0.0;

    // Skip notes for which NoteEvent::isDrum() is true before grouping.
    bool excludeDrums = true;

    // How many ranked candidates to keep per segment at most.
    int maxCandidates = 4;
};

/**
 * Groups the notes by onset and names the chord each group forms.
 *
 * The input is taken by const reference and never modified; the function makes its own working
 * copy, so passing TranscriptionManager::getNoteEventVector() directly is safe.
 *
 * @param inNotes NeuralNote note events, in any order (seconds-based times).
 * @param inOptions Grouping and recognition tunables.
 * @return One ChordSegment per onset group, in time order.
 */
std::vector<ChordSegment> analyzeChords(const std::vector<NoteEvent>& inNotes,
                                        const ChordAnalyzerOptions& inOptions = {});

} // namespace RAL::Analysis

#endif // RAL_ChordAnalyzer_h
