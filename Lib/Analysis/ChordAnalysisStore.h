//
// RAL NeuralNote Lab — Phase 2C
// Chord-analysis result store.
//
// Holds the chord segments derived from the latest note collection and keeps them in step with it.
// This is the small stateful piece that connects RAL::Analysis::analyzeChords to NeuralNote's
// transcription lifecycle: TranscriptionManager owns one and calls update() whenever its notes
// change and clear() when the transcription is cleared.
//
// It is deliberately free of JUCE and of the transcription engine, so it can be unit-tested on its
// own and reused by a future standalone plugin. All methods are meant to be called from one thread
// (NeuralNote's message thread), the same ownership convention as the note vector it mirrors.
//

#ifndef RAL_ChordAnalysisStore_h
#define RAL_ChordAnalysisStore_h

#include <cstdint>
#include <vector>

#include "ChordAnalyzer.h"
#include "NoteEvent.h"

namespace RAL::Analysis
{

class ChordAnalysisStore
{
public:
    /**
     * Recomputes the held segments from `inNotes`, tagging them with `inSession`.
     *
     * `inSession` identifies the transcription run the notes came from and only ever increases.
     * An update whose session is older than the newest one already applied is ignored, so a late
     * result from a previous run can never replace a newer one. Equal sessions are applied, which
     * is how a run's incremental chunks refresh its own results.
     *
     * `inNotes` is read through analyzeChords, which copies it; the caller's notes are not touched.
     *
     * @return true if the held segments were replaced, false if the update was ignored as stale.
     */
    bool update(std::uint64_t inSession,
                const std::vector<NoteEvent>& inNotes,
                const ChordAnalyzerOptions& inOptions = {});

    /** Drops all held segments. The session high-water mark is kept, so a stale update that was in
     *  flight for an already-finished run still cannot repopulate it. */
    void clear();

    /** The latest segments. Reference valid until the next update()/clear(); message thread only. */
    const std::vector<ChordSegment>& segments() const { return mSegments; }

    bool hasResults() const { return !mSegments.empty(); }

    /** The session the currently-held (or last-applied) results belong to. */
    std::uint64_t currentSession() const { return mSession; }

private:
    std::vector<ChordSegment> mSegments;
    std::uint64_t mSession = 0;
};

} // namespace RAL::Analysis

#endif // RAL_ChordAnalysisStore_h
