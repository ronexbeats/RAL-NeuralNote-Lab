//
// RAL NeuralNote Lab — Phase 2C
// Chord-analysis result store implementation.
//

#include "ChordAnalysisStore.h"

namespace RAL::Analysis
{

bool ChordAnalysisStore::update(std::uint64_t inSession,
                                const std::vector<NoteEvent>& inNotes,
                                const ChordAnalyzerOptions& inOptions)
{
    // Older session than the one already applied: a stale result, dropped.
    if (inSession < mSession) {
        return false;
    }

    mSegments = analyzeChords(inNotes, inOptions);
    mSession = inSession;
    return true;
}

void ChordAnalysisStore::clear()
{
    mSegments.clear();
    // mSession is intentionally left as the high-water mark: clearing the display does not lower
    // the bar that keeps an older run's late result out.
}

} // namespace RAL::Analysis
