//
// RAL NeuralNote Lab — Phase 2D
// Pure geometry for the chord-label lane.
//
// Converts chord segments (seconds) into label rectangles (pixels) on the timeline's own
// horizontal axis, using the same pixels-per-second the piano roll uses. Kept free of JUCE so the
// placement rules -- where a label sits, how wide it is, whether it is too narrow to carry text --
// are deterministic and unit-testable without a GUI. The component does the drawing; this decides
// the layout.
//

#ifndef RAL_ChordLabelLayout_h
#define RAL_ChordLabelLayout_h

#include <string>
#include <vector>

#include "ChordAnalyzer.h"

namespace RAL::Analysis
{

/** One chord region placed on the timeline. x/width are in pixels on the lane's own axis (before
 *  any viewport scroll offset, which the component/viewport applies). */
struct ChordLabel {
    double startTime = 0.0; // seconds, from the segment
    double endTime = 0.0; // seconds, from the segment
    float x = 0.0f; // left edge in pixels
    float width = 0.0f; // region width in pixels
    std::string text; // the best candidate's name
    bool ambiguous = false; // segment had more than one reading
    bool labelFits = true; // region is wide enough to draw the text; otherwise draw only a marker
};

/**
 * Lays out labels for the segments that name a chord. Segments with no reliable interpretation are
 * skipped (no label), so the lane never shows a forced or blank name.
 *
 * @param inSegments           Chord segments, in time order (as analyzeChords returns them).
 * @param inPixelsPerSecond    The timeline scale, i.e. basePixelsPerSecond * zoom.
 * @param inMinLabelWidthPixels Regions narrower than this are marked labelFits = false, so the
 *                             caller can draw a boundary marker instead of unreadable clipped text.
 * @return One ChordLabel per named segment, in time order.
 */
std::vector<ChordLabel> layoutChordLabels(const std::vector<ChordSegment>& inSegments,
                                          double inPixelsPerSecond,
                                          float inMinLabelWidthPixels);

} // namespace RAL::Analysis

#endif // RAL_ChordLabelLayout_h
