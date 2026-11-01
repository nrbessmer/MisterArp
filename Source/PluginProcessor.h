/*
  ==============================================================================
    Mister Arp - PluginProcessor.h
    (c)(p) 2026 Tully EDM Vibe
  ==============================================================================
*/
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "TullySynth.h"
#include "SynthParams.h"
#include "TullyUI.h"
#include "Arp.h"
#include "ArpPresets.h"

class MisterArpProcessor : public juce::AudioProcessor
{
public:
    MisterArpProcessor();
    ~MisterArpProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Mister Arp"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    int getNumPrograms() override { return (int) marp::presets().size(); }
    int getCurrentProgram() override { return currentPreset; }
    void setCurrentProgram (int i) override { loadPreset (i); }
    const juce::String getProgramName (int i) override { return juce::isPositiveAndBelow (i, getNumPrograms()) ? marp::presets()[(size_t) i].name : ""; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void loadPreset (int index);
    int  getCurrentPreset() const noexcept { return currentPreset; }

    // 16-step pattern (message thread writes, audio thread reads; each step is one atomic word)
    marp::Step getStep (int i) const noexcept;
    void setStep (int i, const marp::Step& s) noexcept;
    void setPatternFromCode (const juce::String& code);
    juce::String patternToCode() const;

    // current arp settings (any thread)
    marp::Settings currentSettings() const;
    // writes `bars` of the current pattern to a MIDI file; `usedDefault` = no chord was held
    juce::File exportMidi (int bars, const juce::File& file, bool& usedDefault);

    juce::AudioProcessorValueTreeState apvts;
    tsyn::Engine engine;
    marp::Player arp;
    juce::MidiKeyboardState keyboardState;
    tui::ScopeBuffer scope;
    std::atomic<double> tempo { 128.0 };
    std::atomic<bool> hostTempo { false };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    tsyn::ParamReader reader;
    std::array<std::atomic<uint32_t>, marp::kMaxSteps> steps;
    std::atomic<float>* pOn {}; std::atomic<float>* pMode {}; std::atomic<float>* pRate {}; std::atomic<float>* pOct {};
    std::atomic<float>* pGate {}; std::atomic<float>* pSwing {}; std::atomic<float>* pSteps {}; std::atomic<float>* pLatch {};
    std::atomic<float>* pKey {}; std::atomic<float>* pScale {}; std::atomic<float>* pVelKeys {}; std::atomic<float>* pBpm {};
    bool wasOn = true;
    juce::MidiBuffer keysIn, arpOut, synthMidi;
    juce::SpinLock chordLock;
    std::vector<marp::HeldNote> exportChord;
    int currentPreset = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MisterArpProcessor)
};
