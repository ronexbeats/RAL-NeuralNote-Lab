//
// RAL NeuralNote Lab — Phase 2A
// Chord analysis engine implementation.
//

#include "ChordAnalyzer.h"

#include <algorithm>
#include <array>

namespace RAL::Analysis
{

namespace
{

// Pitch-class names, sharps throughout. Index == pitch class 0-11.
constexpr std::array<const char*, 12> kPitchClassNames =
    {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

// A chord template: the pitch-class intervals above the root, in stacked-thirds order (ascending),
// so the index of an interval is also its inversion number (0 = root, 1 = first, ...).
struct ChordTemplate {
    ChordQuality quality;
    std::vector<int> intervals; // e.g. {0, 4, 7} for a major triad
};

// Order matters: it is the tie-break order when several qualities are equally plausible, and it
// puts the more common readings first.
const std::vector<ChordTemplate>& templates()
{
    static const std::vector<ChordTemplate> kTemplates = {
        {ChordQuality::MajorTriad, {0, 4, 7}},
        {ChordQuality::MinorTriad, {0, 3, 7}},
        {ChordQuality::DiminishedTriad, {0, 3, 6}},
        {ChordQuality::AugmentedTriad, {0, 4, 8}},
        {ChordQuality::DominantSeventh, {0, 4, 7, 10}},
        {ChordQuality::MajorSeventh, {0, 4, 7, 11}},
        {ChordQuality::MinorSeventh, {0, 3, 7, 10}},
        {ChordQuality::HalfDiminishedSeventh, {0, 3, 6, 10}},
        {ChordQuality::FullyDiminishedSeventh, {0, 3, 6, 9}},
    };
    return kTemplates;
}

int pitchClassOf(int inMidiPitch)
{
    return ((inMidiPitch % 12) + 12) % 12;
}

// The set of pitch classes a template occupies when rooted at inRoot, sorted ascending so it can be
// compared directly with a segment's sorted pitch-class vector.
std::vector<int> transposedPitchClasses(const ChordTemplate& inTemplate, int inRoot)
{
    std::vector<int> pcs;
    pcs.reserve(inTemplate.intervals.size());
    for (int interval: inTemplate.intervals) {
        pcs.push_back((inRoot + interval) % 12);
    }
    std::sort(pcs.begin(), pcs.end());
    return pcs;
}

std::string makeChordName(int inRootPc, ChordQuality inQuality, int inBassPc, int inInversion)
{
    std::string name = kPitchClassNames[static_cast<size_t>(inRootPc)];
    name += chordQualitySuffix(inQuality);
    if (inInversion != 0 && inBassPc != inRootPc) {
        name += "/";
        name += kPitchClassNames[static_cast<size_t>(inBassPc)];
    }
    return name;
}

// Builds the ranked candidate list for one set of pitch classes and a known bass.
std::vector<ChordCandidate> recognize(const std::vector<int>& inPitchClasses, int inBassPc, int inMaxCandidates)
{
    std::vector<ChordCandidate> candidates;

    const auto numPcs = static_cast<int>(inPitchClasses.size());

    for (const ChordTemplate& tmpl: templates()) {
        // Exact match only: a triad needs exactly three distinct pitch classes, a seventh exactly
        // four. This is what keeps the engine from forcing a name onto an incomplete or extended
        // voicing it does not actually recognise.
        if (chordQualityToneCount(tmpl.quality) != numPcs) {
            continue;
        }

        for (int root = 0; root < 12; ++root) {
            if (transposedPitchClasses(tmpl, root) != inPitchClasses) {
                continue;
            }

            // The bass is one of the sounding pitch classes, hence one of the template's tones:
            // its position in the stacked-thirds interval list is the inversion number.
            const int bassInterval = ((inBassPc - root) % 12 + 12) % 12;
            int inversion = 0;
            for (size_t i = 0; i < tmpl.intervals.size(); ++i) {
                if (tmpl.intervals[i] == bassInterval) {
                    inversion = static_cast<int>(i);
                    break;
                }
            }

            ChordCandidate c;
            c.rootPitchClass = root;
            c.quality = tmpl.quality;
            c.bassPitchClass = inBassPc;
            c.inversion = inversion;
            c.name = makeChordName(root, tmpl.quality, inBassPc, inversion);
            candidates.push_back(std::move(c));
        }
    }

    // Confidence: a single exact match is certain; competing roots (a symmetric chord such as an
    // augmented triad or a fully diminished seventh) split confidence evenly.
    const double confidence = candidates.empty() ? 0.0 : 1.0 / static_cast<double>(candidates.size());
    for (ChordCandidate& c: candidates) {
        c.confidence = confidence;
    }

    // Rank: root-position reading first (its root is the sounding bass), then by the fixed template
    // order, then by root pitch class. Fully deterministic.
    std::sort(candidates.begin(), candidates.end(), [](const ChordCandidate& a, const ChordCandidate& b) {
        const bool aRootPos = a.rootPitchClass == a.bassPitchClass;
        const bool bRootPos = b.rootPitchClass == b.bassPitchClass;
        if (aRootPos != bRootPos) {
            return aRootPos; // root position sorts first
        }
        if (a.quality != b.quality) {
            return static_cast<int>(a.quality) < static_cast<int>(b.quality);
        }
        return a.rootPitchClass < b.rootPitchClass;
    });

    if (static_cast<int>(candidates.size()) > inMaxCandidates && inMaxCandidates > 0) {
        candidates.resize(static_cast<size_t>(inMaxCandidates));
    }

    return candidates;
}

} // namespace

const char* chordQualitySuffix(ChordQuality inQuality)
{
    switch (inQuality) {
        case ChordQuality::MajorTriad:
            return "";
        case ChordQuality::MinorTriad:
            return "m";
        case ChordQuality::DiminishedTriad:
            return "dim";
        case ChordQuality::AugmentedTriad:
            return "aug";
        case ChordQuality::DominantSeventh:
            return "7";
        case ChordQuality::MajorSeventh:
            return "maj7";
        case ChordQuality::MinorSeventh:
            return "m7";
        case ChordQuality::HalfDiminishedSeventh:
            return "m7b5";
        case ChordQuality::FullyDiminishedSeventh:
            return "dim7";
    }
    return "";
}

int chordQualityToneCount(ChordQuality inQuality)
{
    switch (inQuality) {
        case ChordQuality::MajorTriad:
        case ChordQuality::MinorTriad:
        case ChordQuality::DiminishedTriad:
        case ChordQuality::AugmentedTriad:
            return 3;
        case ChordQuality::DominantSeventh:
        case ChordQuality::MajorSeventh:
        case ChordQuality::MinorSeventh:
        case ChordQuality::HalfDiminishedSeventh:
        case ChordQuality::FullyDiminishedSeventh:
            return 4;
    }
    return 0;
}

std::vector<ChordSegment> analyzeChords(const std::vector<NoteEvent>& inNotes, const ChordAnalyzerOptions& inOptions)
{
    // Work on our own copy; the caller's transcription data is never touched.
    std::vector<NoteEvent> notes;
    notes.reserve(inNotes.size());
    for (const NoteEvent& n: inNotes) {
        if (inOptions.excludeDrums && n.isDrum()) {
            continue;
        }
        // Skip notes outside the valid MIDI range [0, 127]. NeuralNote's own notes are always in
        // range, but a stray value from any future source would otherwise be folded into a pitch
        // class (e.g. -1 -> B) and could fabricate or misname a chord. Dropping it keeps the
        // analysis honest rather than inventing a note.
        if (n.pitch < 0 || n.pitch > 127) {
            continue;
        }
        notes.push_back(n);
    }

    NoteEvent::sortEvents(notes); // by start time, then program, pitch, end

    std::vector<ChordSegment> segments;

    size_t i = 0;
    while (i < notes.size()) {
        // Open an onset group at the next note. Everything whose onset is within the tolerance of
        // this first onset joins it; the tolerance bounds the group's onset spread, so a long
        // arpeggio does not collapse into one chord through drift.
        const double groupOnset = notes[i].startTime;
        size_t j = i;
        double segStart = notes[i].startTime;
        double segEnd = notes[i].endTime;

        std::vector<int> pitches;
        while (j < notes.size() && notes[j].startTime - groupOnset <= inOptions.onsetToleranceSeconds) {
            pitches.push_back(notes[j].pitch);
            segStart = std::min(segStart, notes[j].startTime);
            segEnd = std::max(segEnd, notes[j].endTime);
            ++j;
        }
        i = j;

        if (inOptions.minSegmentDurationSeconds > 0.0
            && (segEnd - segStart) < inOptions.minSegmentDurationSeconds) {
            continue;
        }

        ChordSegment seg;
        seg.startTime = segStart;
        seg.endTime = segEnd;

        std::sort(pitches.begin(), pitches.end());
        seg.pitches = pitches;

        // Fold octaves into a sorted set of pitch classes; this is what the lowest note's bass and
        // the templates are compared on.
        std::vector<int> pcs;
        for (int p: pitches) {
            pcs.push_back(pitchClassOf(p));
        }
        std::sort(pcs.begin(), pcs.end());
        pcs.erase(std::unique(pcs.begin(), pcs.end()), pcs.end());
        seg.pitchClasses = pcs;

        const int bassPc = pitchClassOf(pitches.front()); // pitches is sorted; front is lowest
        seg.candidates = recognize(pcs, bassPc, inOptions.maxCandidates);
        seg.isAmbiguous = seg.candidates.size() > 1;

        segments.push_back(std::move(seg));
    }

    return segments;
}

} // namespace RAL::Analysis
