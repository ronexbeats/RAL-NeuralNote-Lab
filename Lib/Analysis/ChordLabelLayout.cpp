//
// RAL NeuralNote Lab — Phase 2D
// Pure geometry for the chord-label lane.
//

#include "ChordLabelLayout.h"

#include <algorithm>

namespace RAL::Analysis
{

std::vector<ChordLabel> layoutChordLabels(const std::vector<ChordSegment>& inSegments,
                                          double inPixelsPerSecond,
                                          float inMinLabelWidthPixels)
{
    std::vector<ChordLabel> labels;
    labels.reserve(inSegments.size());

    for (const ChordSegment& seg: inSegments) {
        const ChordCandidate* best = seg.best();

        // No reliable chord: no label. The lane shows nothing rather than a forced name.
        if (best == nullptr) {
            continue;
        }

        ChordLabel label;
        label.startTime = seg.startTime;
        label.endTime = seg.endTime;
        label.x = static_cast<float>(seg.startTime * inPixelsPerSecond);
        label.width = static_cast<float>(std::max(0.0, (seg.endTime - seg.startTime) * inPixelsPerSecond));
        label.text = best->name;
        label.ambiguous = seg.isAmbiguous;
        label.labelFits = label.width >= inMinLabelWidthPixels;

        labels.push_back(std::move(label));
    }

    return labels;
}

} // namespace RAL::Analysis
