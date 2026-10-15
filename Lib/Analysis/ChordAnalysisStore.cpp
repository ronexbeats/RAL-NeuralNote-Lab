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

    // Same session as the held/last results, but the store was cleared since: the run that owned
    // this session is gone, so a late result carrying its id must not repopulate it. Only a
    // strictly newer run may. This is what makes clear() final against an in-flight same-session
    // update, without breaking the equal-session incremental refresh of a live run.
    if (inSession == mSession && mClearedSinceApply) {
        return false;
    }

    mSegments = analyzeChords(inNotes, inOptions);
    mSession = inSession;
    mClearedSinceApply = false;
    return true;
}

void ChordAnalysisStore::clear()
{
    mSegments.clear();
    // mSession is kept as the high-water mark and the store is marked cleared, so neither an older
    // nor an equal-session update can repopulate it -- only a strictly newer run.
    mClearedSinceApply = true;
}

} // namespace RAL::Analysis
