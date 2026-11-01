/*
  ==============================================================================
    Mister Arp - PluginProcessor.cpp
    (c)(p) 2026 Tully EDM Vibe
  ==============================================================================
*/
#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    // step <-> 32-bit word: on 1 | tie 1 | ratchet-1 2 | trans+12 5 | vel 7 | prob 7
    uint32_t pack (const marp::Step& s)
    {
        const uint32_t v = (uint32_t) juce::jlimit (0, 127, juce::roundToInt (s.vel * 127.0f));
        const uint32_t p = (uint32_t) juce::jlimit (0, 127, juce::roundToInt (s.prob * 127.0f));
        return (s.on ? 1u : 0u) | (s.tie ? 2u : 0u) | ((uint32_t) (juce::jlimit (1, 4, s.ratchet) - 1) << 2)
             | ((uint32_t) (juce::jlimit (-12, 12, s.trans) + 12) << 4) | (v << 9) | (p << 16);
    }
    marp::Step unpack (uint32_t w)
    {
        marp::Step s;
        s.on = (w & 1u) != 0; s.tie = (w & 2u) != 0; s.ratchet = (int) ((w >> 2) & 3u) + 1;
        s.trans = (int) ((w >> 4) & 31u) - 12; s.vel = (float) ((w >> 9) & 127u) / 127.0f; s.prob = (float) ((w >> 16) & 127u) / 127.0f;
        return s;
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout MisterArpProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout l;
    tsyn::addSynthParameters (l);
    auto pct = AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return String (roundToInt (v * 100.0f)) + "%"; });
    l.add (std::make_unique<AudioParameterBool> (ParameterID { "arp_on", 1 }, "Arp On", true));
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "arp_mode", 1 }, "Arp Mode", marp::modeNames(), 0));
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "arp_rate", 1 }, "Arp Rate", marp::rateNames(), 2));
    l.add (std::make_unique<AudioParameterInt> (ParameterID { "arp_oct", 1 }, "Arp Octaves", 1, 4, 1));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "arp_gate", 1 }, "Arp Gate", NormalisableRange<float> (0.05f, 1.5f), 0.6f, pct));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "arp_swing", 1 }, "Arp Swing", NormalisableRange<float> (0.0f, 1.0f), 0.0f, pct));
    l.add (std::make_unique<AudioParameterInt> (ParameterID { "arp_steps", 1 }, "Arp Steps", 1, 16, 16));
    l.add (std::make_unique<AudioParameterBool> (ParameterID { "arp_latch", 1 }, "Arp Latch", false));
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "arp_key", 1 }, "Arp Key", marp::keyNames(), 0));
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "arp_scale", 1 }, "Arp Scale", marp::scaleNames(), 0));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "arp_velkeys", 1 }, "Arp Key Velocity", NormalisableRange<float> (0.0f, 1.0f), 0.0f, pct));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "arp_bpm", 1 }, "Arp Tempo", NormalisableRange<float> (60.0f, 200.0f, 1.0f), 128.0f,
                                                  AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return String (roundToInt (v)) + " BPM"; })));
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "arp_bars", 1 }, "MIDI Bars", StringArray { "1 bar", "2 bars", "4 bars", "8 bars" }, 2));
    return l;
}

MisterArpProcessor::MisterArpProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
    reader.attach (apvts);
    pOn = apvts.getRawParameterValue ("arp_on"); pMode = apvts.getRawParameterValue ("arp_mode"); pRate = apvts.getRawParameterValue ("arp_rate");
    pOct = apvts.getRawParameterValue ("arp_oct"); pGate = apvts.getRawParameterValue ("arp_gate"); pSwing = apvts.getRawParameterValue ("arp_swing");
    pSteps = apvts.getRawParameterValue ("arp_steps"); pLatch = apvts.getRawParameterValue ("arp_latch"); pKey = apvts.getRawParameterValue ("arp_key");
    pScale = apvts.getRawParameterValue ("arp_scale"); pVelKeys = apvts.getRawParameterValue ("arp_velkeys"); pBpm = apvts.getRawParameterValue ("arp_bpm");
    for (auto& s : steps) s.store (pack (marp::Step {}));
    loadPreset (0);
}

bool MisterArpProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto out = l.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

void MisterArpProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, juce::jmax (1, samplesPerBlock));
    arp.prepare (sampleRate);
    keyboardState.reset();
    keysIn.ensureSize (2048); arpOut.ensureSize (2048); synthMidi.ensureSize (4096);
}

marp::Step MisterArpProcessor::getStep (int i) const noexcept { return unpack (steps[(size_t) juce::jlimit (0, marp::kMaxSteps - 1, i)].load()); }
void MisterArpProcessor::setStep (int i, const marp::Step& s) noexcept { steps[(size_t) juce::jlimit (0, marp::kMaxSteps - 1, i)].store (pack (s)); }

marp::Settings MisterArpProcessor::currentSettings() const
{
    marp::Settings s;
    s.mode = (int) pMode->load(); s.stepBeats = marp::rateBeats ((int) pRate->load()); s.octaves = (int) pOct->load();
    s.gate = pGate->load(); s.swing = pSwing->load(); s.numSteps = (int) pSteps->load();
    s.key = (int) pKey->load(); s.scale = (int) pScale->load(); s.velFromKeys = pVelKeys->load();
    for (int i = 0; i < marp::kMaxSteps; ++i) s.steps[(size_t) i] = getStep (i);
    return s;
}

void MisterArpProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals nd;
    const int n = buffer.getNumSamples();
    const int nch = buffer.getNumChannels();
    if (n == 0 || nch == 0) return;

    double bpm = pBpm->load(), ppq = 0.0; bool playing = false, hasTempo = false;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm()) { bpm = *b; hasTempo = true; }
            if (auto p = pos->getPpqPosition()) ppq = *p; else hasTempo = hasTempo && false;
            playing = pos->getIsPlaying() && pos->getPpqPosition().hasValue();
        }
    tempo.store (bpm); hostTempo.store (hasTempo);

    tsyn::Params p; tsyn::FxParams f;
    const double lfoBeats = reader.read (p, f, bpm);
    engine.setParams (p, f);
    engine.setLfoSync (lfoBeats > 0.0 && playing, ppq, lfoBeats);

    keyboardState.processNextMidiBuffer (midi, 0, n, true);

    const bool on = pOn->load() > 0.5f;
    synthMidi.clear();
    if (on)
    {
        keysIn.clear(); arpOut.clear();
        for (const auto m : midi)
        {
            const auto msg = m.getMessage();
            if (msg.isNoteOnOrOff() || msg.isAllNotesOff() || msg.isAllSoundOff()) keysIn.addEvent (msg, m.samplePosition);
            else synthMidi.addEvent (msg, m.samplePosition);
        }
        arp.process (keysIn, arpOut, n, currentSettings(), pLatch->load() > 0.5f, playing, ppq, bpm);
        synthMidi.addEvents (arpOut, 0, n, 0);
        const juce::SpinLock::ScopedTryLockType lock (chordLock);
        if (lock.isLocked()) { arp.rememberChord(); exportChord = arp.notesForExport (pLatch->load() > 0.5f); }
    }
    else
    {
        if (wasOn) { arpOut.clear(); arp.flushAll (arpOut, 0); synthMidi.addEvents (arpOut, 0, n, 0); arp.reset(); }
        synthMidi.addEvents (midi, 0, n, 0);
    }
    wasOn = on;

    float* L = buffer.getWritePointer (0);
    float* R = nch > 1 ? buffer.getWritePointer (1) : nullptr;
    juce::HeapBlock<float> monoR;
    if (R == nullptr) { monoR.allocate ((size_t) n, true); R = monoR.get(); }
    int pos = 0;
    for (const auto meta : synthMidi)
    {
        const int at = juce::jlimit (0, n, meta.samplePosition);
        if (at > pos) { engine.render (L + pos, R + pos, at - pos); pos = at; }
        engine.handleMidi (meta.getMessage());
    }
    if (pos < n) engine.render (L + pos, R + pos, n - pos);
    if (nch == 1) for (int i = 0; i < n; ++i) L[i] = 0.5f * (L[i] + R[i]);
    for (int c = 2; c < nch; ++c) buffer.clear (c, 0, n);
    scope.push (L, nch > 1 ? R : L, n);
    midi.clear();
}

juce::File MisterArpProcessor::exportMidi (int bars, const juce::File& file, bool& usedDefault)
{
    std::vector<marp::HeldNote> notes;
    { const juce::SpinLock::ScopedLockType lock (chordLock); notes = exportChord; }
    usedDefault = notes.empty();
    if (usedDefault) notes = { { 48, 0.8f, 0 }, { 51, 0.8f, 1 }, { 55, 0.8f, 2 } };   // C minor
    return marp::exportMidi (notes, currentSettings(), bars, tempo.load(), file);
}

// ---- pattern code: steps separated by ','  e.g. "X,x,o,.,x_,_,xr2,x+12p50"
//      x = on (vel .75)  X = accent (1.0)  o = soft (.5)  . = rest  _ = tie
//      rN = ratchet N   pNN = probability %   +N / -N = transpose
void MisterArpProcessor::setPatternFromCode (const juce::String& code)
{
    auto toks = juce::StringArray::fromTokens (code, ",", "");
    for (int i = 0; i < marp::kMaxSteps; ++i)
    {
        marp::Step s;
        const juce::String t = i < toks.size() ? toks[i].trim() : juce::String ("x");
        if (t.isEmpty()) { setStep (i, s); continue; }
        const auto c = t[0];
        s.on = c != '.'; s.tie = c == '_';
        s.vel = c == 'X' ? 1.0f : c == 'o' ? 0.5f : 0.75f;
        for (int k = 1; k < t.length(); ++k)
        {
            const auto m = t[k];
            if (m == 'r') s.ratchet = juce::jlimit (1, 4, t.substring (k + 1).getIntValue());
            else if (m == 'p') s.prob = juce::jlimit (0.0f, 1.0f, (float) t.substring (k + 1).getIntValue() / 100.0f);
            else if (m == '+') s.trans = juce::jlimit (-12, 12, t.substring (k + 1).getIntValue());
            else if (m == '-') s.trans = juce::jlimit (-12, 12, -t.substring (k + 1).getIntValue());
            else if (m == '_') s.tie = true;
            else if (m == 'v') s.vel = juce::jlimit (0.05f, 1.0f, (float) t.substring (k + 1).getIntValue() / 100.0f);
        }
        setStep (i, s);
    }
}

juce::String MisterArpProcessor::patternToCode() const
{
    juce::StringArray out;
    for (int i = 0; i < marp::kMaxSteps; ++i)
    {
        const auto s = getStep (i);
        juce::String t = ! s.on ? "." : s.tie ? "_" : s.vel > 0.9f ? "X" : s.vel < 0.6f ? "o" : "x";
        if (s.on && ! s.tie && std::abs (s.vel - 0.75f) > 0.02f && std::abs (s.vel - 1.0f) > 0.02f && std::abs (s.vel - 0.5f) > 0.02f)
            t << "v" << juce::roundToInt (s.vel * 100.0f);
        if (s.ratchet > 1) t << "r" << s.ratchet;
        if (s.prob < 0.99f) t << "p" << juce::roundToInt (s.prob * 100.0f);
        if (s.trans > 0) t << "+" << s.trans; else if (s.trans < 0) t << s.trans;
        out.add (t);
    }
    return out.joinIntoString (",");
}

void MisterArpProcessor::loadPreset (int index)
{
    if (! juce::isPositiveAndBelow (index, (int) marp::presets().size())) return;
    auto extra = tsyn::applyPreset (apvts, marp::presets()[(size_t) index], { "arp_bpm", "arp_bars", "arp_on", "arp_latch" });
    setPatternFromCode (extra.containsKey ("steps") ? extra["steps"] : juce::String());
    currentPreset = index;
}

void MisterArpProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto st = apvts.copyState();
    st.setProperty ("preset", currentPreset, nullptr);
    juce::StringArray words; for (auto& s : steps) words.add (juce::String ((juce::int64) s.load()));
    st.setProperty ("steps", words.joinIntoString (" "), nullptr);
    if (auto xml = st.createXml()) copyXmlToBinary (*xml, dest);
}

void MisterArpProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto st = juce::ValueTree::fromXml (*xml);
            currentPreset = juce::jlimit (0, getNumPrograms() - 1, (int) st.getProperty ("preset", 0));
            auto words = juce::StringArray::fromTokens (st.getProperty ("steps").toString(), " ", "");
            for (int i = 0; i < marp::kMaxSteps && i < words.size(); ++i) steps[(size_t) i].store ((uint32_t) words[i].getLargeIntValue());
            apvts.replaceState (st);
        }
}

juce::AudioProcessorEditor* MisterArpProcessor::createEditor() { return new MisterArpEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MisterArpProcessor(); }
