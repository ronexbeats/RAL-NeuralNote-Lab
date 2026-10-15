//
// RAL NeuralNote Lab — Phase 2D
// Read-only chord-label lane implementation.
//

#include "ChordTimeline.h"

#include "ChordLabelLayout.h"
#include "NnFonts.h"
#include "NnLook.h"
#include "PluginProcessor.h"

namespace
{
constexpr int LABEL_INSET = 6;
// A region narrower than this cannot carry a legible name, so only its boundary is drawn. Roughly
// the width of a two-glyph chord name at the lane's font; the drawText ellipsis handles the rest.
constexpr float MIN_LABEL_WIDTH = 26.0f;
} // namespace

ChordTimeline::ChordTimeline(NeuralNoteAudioProcessor* inProcessor, double inBaseNumPixelsPerSecond)
    : mProcessor(inProcessor)
    , mBaseNumPixelsPerSecond(inBaseNumPixelsPerSecond)
{
    // The mixer broadcasts when the transcription's notes are rebuilt and when they are cleared,
    // which is exactly when the chord results change; the same signal the piano roll follows.
    mProcessor->getInstrumentMixer()->addChangeListener(this);
}

ChordTimeline::~ChordTimeline()
{
    mProcessor->getInstrumentMixer()->removeChangeListener(this);
}

void ChordTimeline::setZoomLevel(double inZoomLevel)
{
    mZoomLevel = inZoomLevel;
    repaint();
}

void ChordTimeline::changeListenerCallback(juce::ChangeBroadcaster* inSource)
{
    juce::ignoreUnused(inSource);
    repaint();
}

void ChordTimeline::paint(juce::Graphics& g)
{
    g.fillAll(nn::colours::bgPanel);
    nn::drawBottomBorder(g, getLocalBounds(), nn::colours::divSoft);

    if (!mProcessor->canPlay()) {
        return;
    }

    const double pixels_per_second = mBaseNumPixelsPerSecond * mZoomLevel;
    const auto& segments = mProcessor->getTranscriptionManager()->getChordSegments();
    const auto labels = RAL::Analysis::layoutChordLabels(segments, pixels_per_second, MIN_LABEL_WIDTH);

    g.setFont(nn::fonts::buttonLabel());

    const int height = getHeight();

    for (const RAL::Analysis::ChordLabel& label: labels) {
        const int x = static_cast<int>(std::round(label.x));

        // A subtle boundary at the start of every chord region, named or not wide enough to name.
        g.setColour(nn::colours::divTick);
        g.fillRect(x, 0, 1, height);

        // Too narrow for readable text: the boundary alone marks the region.
        if (!label.labelFits) {
            continue;
        }

        // Ambiguous readings (a symmetric chord with competing roots) are shown dimmed with a
        // trailing "?" -- present but visibly less certain, rather than asserted as fact.
        g.setColour(label.ambiguous ? nn::colours::textFaint : nn::colours::textButton);

        const int text_x = x + LABEL_INSET;
        const int text_w = static_cast<int>(std::round(label.width)) - LABEL_INSET;
        const juce::String text = juce::String(label.text) + (label.ambiguous ? " ?" : "");

        g.drawText(text,
                   juce::Rectangle<int>(text_x, 0, std::max(1, text_w), height),
                   juce::Justification::centredLeft,
                   /* useEllipsesIfTooBig */ true);
    }
}
