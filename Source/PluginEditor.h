/*
  ==============================================================================
    Mister Arp - PluginEditor.h
    (c)(p) 2026 Tully EDM Vibe
  ==============================================================================
*/
#pragma once
#include "PluginProcessor.h"

namespace marpui
{
    // 16 columns x (step / velocity / ratchet / probability / note / tie)
    class StepGrid : public juce::Component, public juce::TooltipClient
    {
    public:
        explicit StepGrid (MisterArpProcessor& p) : proc (p) {}
        void paint (juce::Graphics&) override;
        void mouseDown (const juce::MouseEvent&) override;
        void mouseDrag (const juce::MouseEvent&) override;
        void mouseUp (const juce::MouseEvent&) override { dragRow = -1; }
        void mouseDoubleClick (const juce::MouseEvent&) override;
        void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
        juce::String getTooltip() override;
        void setPlayingStep (int s) { if (s != playing) { playing = s; repaint(); } }
        enum Row { RStep = 0, RVel, RRatchet, RProb, RNote, RTie, kRows };
    private:
        juce::Rectangle<int> cell (int col, int row) const;
        int colAt (int x) const;
        int rowAt (int y) const;
        void apply (int col, int row, const juce::MouseEvent& e, bool first);
        MisterArpProcessor& proc;
        int playing = -1, dragRow = -1, dragStartTrans = 0, dragStartY = 0;
        bool paintOn = true;
        static constexpr int kLabelW = 78;
    };

    // Drag this into a DAW track to get the arp as MIDI; click to save it to Music/Mister Arp.
    class DragMidi : public juce::Component, public juce::SettableTooltipClient
    {
    public:
        explicit DragMidi (MisterArpProcessor& p) : proc (p)
        { setTooltip ("Drag onto a track in your DAW to drop the arp as MIDI (uses the chord you're holding or last held). Click to save the file to Music/Mister Arp."); }
        void paint (juce::Graphics&) override;
        void mouseDown (const juce::MouseEvent&) override { dragged = false; }
        void mouseDrag (const juce::MouseEvent&) override;
        void mouseUp (const juce::MouseEvent&) override;
        void mouseEnter (const juce::MouseEvent&) override { repaint(); }
        void mouseExit (const juce::MouseEvent&) override { repaint(); }
        std::function<void (const juce::String&)> onStatus;
    private:
        juce::File make (const juce::File& dir);
        MisterArpProcessor& proc;
        bool dragged = false;
    };
}

class MisterArpEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit MisterArpEditor (MisterArpProcessor&);
    ~MisterArpEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;   // public so tests / the video renderer can drive it
    void showPage (int page);

    MisterArpProcessor& proc;
    tui::LnF lnf;
    juce::Image logo;
    tui::PresetBar presetBar;
    tui::Scope scope;
    juce::TextButton arpTab { "ARP" }, soundTab { "SOUND" };
    tui::Toggle arpOn, latch;
    tui::Choice mode, rate, key, scale, bars;
    std::vector<std::unique_ptr<tui::Knob>> knobs;
    tui::Knob *oct = nullptr, *gate = nullptr, *swing = nullptr, *numSteps = nullptr, *velKeys = nullptr, *bpm = nullptr;
    std::vector<tui::Knob*> quick;
    marpui::StepGrid grid;
    marpui::DragMidi dragMidi;
    juce::Label status, tempoLabel;
    tui::SynthPanel panel;
    juce::MidiKeyboardComponent keyboard;
    juce::TooltipWindow tooltips { this, 500 };
    int page = 0;
    juce::Rectangle<int> rPattern, rScale, rQuick, rGrid, rTop;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MisterArpEditor)
};
