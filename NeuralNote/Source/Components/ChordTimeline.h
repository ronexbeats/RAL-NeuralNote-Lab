//
// RAL NeuralNote Lab — Phase 2D
// Read-only chord-label lane shown above the piano roll.
//
// Lives inside the same horizontally scrolling region as the waveform, ruler and piano roll, so it
// shares their time axis and cannot drift from them under scroll or zoom. It only draws: the chord
// names come from TranscriptionManager::getChordSegments(), placed by RAL::Analysis layout. It
// never changes notes, timing or any transcription data.
//

#ifndef ChordTimeline_h
#define ChordTimeline_h

#include <JuceHeader.h>

class NeuralNoteAudioProcessor;

class ChordTimeline
    : public juce::Component
    , public juce::ChangeListener
{
public:
    ChordTimeline(NeuralNoteAudioProcessor* inProcessor, double inBaseNumPixelsPerSecond);

    ~ChordTimeline() override;

    void paint(juce::Graphics& g) override;

    void setZoomLevel(double inZoomLevel);

    /** Repaints when the transcription's notes change or are cleared (via the instrument mixer). */
    void changeListenerCallback(juce::ChangeBroadcaster* inSource) override;

private:
    NeuralNoteAudioProcessor* mProcessor;
    double mBaseNumPixelsPerSecond;
    double mZoomLevel = 1.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChordTimeline)
};

#endif // ChordTimeline_h
