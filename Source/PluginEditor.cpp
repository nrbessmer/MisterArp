/*
  ==============================================================================
    Mister Arp - PluginEditor.cpp
    (c)(p) 2026 Tully EDM Vibe
  ==============================================================================
*/
#include "PluginEditor.h"
#include "BinaryData.h"

using namespace marpui;
namespace Col = tui::Col;

//============================================================================== StepGrid
juce::Rectangle<int> StepGrid::cell (int col, int row) const
{
    const int w = (getWidth() - kLabelW) / marp::kMaxSteps;
    const int heights[kRows] = { 26, 0, 22, 34, 24, 20 };
    int fixed = 0; for (int h : heights) fixed += h;
    const int velH = getHeight() - fixed - 5 * 4;
    int y = 0;
    for (int r = 0; r < row; ++r) y += (r == RVel ? velH : heights[r]) + 4;
    const int h = row == RVel ? velH : heights[row];
    return { kLabelW + col * w + 2, y, w - 4, h };
}

int StepGrid::colAt (int x) const
{
    const int w = (getWidth() - kLabelW) / marp::kMaxSteps;
    return juce::jlimit (0, marp::kMaxSteps - 1, (x - kLabelW) / juce::jmax (1, w));
}

int StepGrid::rowAt (int y) const
{
    for (int r = 0; r < kRows; ++r) { auto c = cell (0, r); if (y >= c.getY() - 2 && y < c.getBottom() + 2) return r; }
    return -1;
}

void StepGrid::paint (juce::Graphics& g)
{
    const int used = (int) proc.apvts.getRawParameterValue ("arp_steps")->load();
    const char* labels[kRows] = { "STEP", "VELOCITY", "RATCHET", "CHANCE", "NOTE", "TIE" };
    g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    for (int r = 0; r < kRows; ++r)
    {
        auto c = cell (0, r);
        g.setColour (Col::dim); g.drawText (labels[r], 0, c.getY(), kLabelW - 8, c.getHeight(), juce::Justification::centredRight);
    }
    for (int i = 0; i < marp::kMaxSteps; ++i)
    {
        const auto s = proc.getStep (i);
        const bool inUse = i < used;
        const float alpha = inUse ? 1.0f : 0.28f;
        const bool hot = i == playing && inUse;
        const bool beat = i % 4 == 0;

        // step on/off
        auto c = cell (i, RStep).toFloat();
        g.setColour ((s.on ? (s.tie ? Col::amber : Col::pink) : Col::panelHi).withAlpha (alpha));
        g.fillRoundedRectangle (c, 4.0f);
        if (hot) { g.setColour (Col::cyan); g.drawRoundedRectangle (c.reduced (0.5f), 4.0f, 2.0f); }
        g.setColour ((s.on ? Col::bg : (beat ? Col::text : Col::dim)).withAlpha (alpha));
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText (juce::String (i + 1), c, juce::Justification::centred);

        // velocity bar
        c = cell (i, RVel).toFloat();
        g.setColour (Col::panelHi.withAlpha (alpha)); g.fillRoundedRectangle (c, 3.0f);
        if (s.on && ! s.tie)
        {
            auto bar = c.withTrimmedTop (c.getHeight() * (1.0f - s.vel));
            g.setGradientFill (juce::ColourGradient (Col::pink.withAlpha (alpha), 0, bar.getY(), Col::pink.withAlpha (0.35f * alpha), 0, c.getBottom(), false));
            g.fillRoundedRectangle (bar, 3.0f);
        }
        if (hot) { g.setColour (Col::cyan.withAlpha (0.25f)); g.fillRoundedRectangle (c, 3.0f); }

        // ratchet
        c = cell (i, RRatchet).toFloat();
        g.setColour (Col::panelHi.withAlpha (alpha)); g.fillRoundedRectangle (c, 3.0f);
        g.setColour ((s.ratchet > 1 ? Col::cyan : Col::dim).withAlpha (alpha));
        const float dw = (c.getWidth() - 6) / 4.0f;
        for (int k = 0; k < s.ratchet; ++k) g.fillRoundedRectangle (c.getX() + 3 + k * dw + 1, c.getY() + 6, dw - 2, c.getHeight() - 12, 1.5f);

        // probability
        c = cell (i, RProb).toFloat();
        g.setColour (Col::panelHi.withAlpha (alpha)); g.fillRoundedRectangle (c, 3.0f);
        g.setColour (Col::violet.withAlpha (alpha * (s.prob < 0.99f ? 1.0f : 0.45f)));
        g.fillRoundedRectangle (c.withTrimmedTop (c.getHeight() * (1.0f - s.prob)), 3.0f);
        g.setColour (Col::text.withAlpha (alpha)); g.setFont (juce::FontOptions (9.5f));
        g.drawText (juce::String (juce::roundToInt (s.prob * 100.0f)), c, juce::Justification::centred);

        // transpose
        c = cell (i, RNote).toFloat();
        g.setColour ((s.trans != 0 ? Col::amber.withAlpha (0.25f) : Col::panelHi).withAlpha (alpha)); g.fillRoundedRectangle (c, 3.0f);
        g.setColour ((s.trans != 0 ? Col::amber : Col::dim).withAlpha (alpha)); g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText (s.trans == 0 ? juce::String ("0") : (s.trans > 0 ? "+" : "") + juce::String (s.trans), c, juce::Justification::centred);

        // tie
        c = cell (i, RTie).toFloat();
        g.setColour ((s.tie ? Col::amber : Col::panelHi).withAlpha (alpha)); g.fillRoundedRectangle (c, 3.0f);
        if (s.tie) { g.setColour (Col::bg); g.drawText ("TIE", c, juce::Justification::centred); }
    }
}

void StepGrid::apply (int col, int row, const juce::MouseEvent& e, bool first)
{
    auto s = proc.getStep (col);
    auto c = cell (col, row);
    const float frac = juce::jlimit (0.0f, 1.0f, 1.0f - (float) (e.y - c.getY()) / (float) juce::jmax (1, c.getHeight()));
    switch (row)
    {
        case RStep:   if (first) paintOn = ! s.on; s.on = paintOn; if (s.on) s.tie = false; break;
        case RVel:    s.vel = juce::jmax (0.05f, frac); if (! s.on) { s.on = true; s.tie = false; } break;
        case RRatchet: if (first) s.ratchet = e.mods.isRightButtonDown() ? (s.ratchet + 2) % 4 + 1 : s.ratchet % 4 + 1; break;
        case RProb:   s.prob = std::round (frac * 20.0f) / 20.0f; break;
        case RNote:
            if (first) { dragStartTrans = s.trans; dragStartY = e.y; }
            s.trans = juce::jlimit (-12, 12, dragStartTrans + (dragStartY - e.y) / 6);
            break;
        case RTie:    if (first) paintOn = ! s.tie; s.tie = paintOn; if (s.tie) s.on = true; break;
        default: return;
    }
    proc.setStep (col, s);
    repaint();
}

void StepGrid::mouseDown (const juce::MouseEvent& e)
{
    if (e.x < kLabelW) return;
    dragRow = rowAt (e.y);
    if (dragRow >= 0) apply (colAt (e.x), dragRow, e, true);
}

void StepGrid::mouseDrag (const juce::MouseEvent& e)
{
    if (dragRow < 0 || dragRow == RRatchet) return;
    if (dragRow == RNote) { apply (colAt (e.getMouseDownX()), dragRow, e, false); return; }
    auto row = dragRow;
    apply (colAt (e.x), row, e.withNewPosition (juce::Point<int> (e.x, juce::jlimit (cell (0, row).getY(), cell (0, row).getBottom(), e.y))), false);
}

void StepGrid::mouseDoubleClick (const juce::MouseEvent& e)
{
    const int row = rowAt (e.y); if (row < 0 || e.x < kLabelW) return;
    const int col = colAt (e.x);
    auto s = proc.getStep (col);
    if (row == RNote) s.trans = 0; else if (row == RProb) s.prob = 1.0f; else if (row == RVel) s.vel = 0.75f; else return;
    proc.setStep (col, s); repaint();
}

void StepGrid::mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& w)
{
    const int row = rowAt (e.y); if (row != RNote || e.x < kLabelW) return;
    const int col = colAt (e.x);
    auto s = proc.getStep (col);
    s.trans = juce::jlimit (-12, 12, s.trans + (w.deltaY > 0 ? 1 : -1));
    proc.setStep (col, s); repaint();
}

juce::String StepGrid::getTooltip()
{
    const auto m = getMouseXYRelative();
    switch (rowAt (m.y))
    {
        case RStep:    return "Click to switch steps on/off (drag across to paint). The cyan outline shows the step playing.";
        case RVel:     return "Velocity: drag up/down (drag across steps to draw a curve). Double-click resets.";
        case RRatchet: return "Ratchet: click to repeat the note 1-4 times inside the step (right-click goes back).";
        case RProb:    return "Chance the step plays (drag). Lower values make the pattern vary every time. Double-click = 100%.";
        case RNote:    return "Transpose this step: drag up/down or scroll (-12..+12 semitones; with a SCALE it snaps to the scale). Double-click = 0.";
        case RTie:     return "Tie: hold the previous note through this step instead of playing a new one.";
        default:       return {};
    }
}

//============================================================================== DragMidi
void DragMidi::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    const bool over = isMouseOver();
    g.setColour (over ? Col::cyan.withAlpha (0.25f) : Col::panelHi); g.fillRoundedRectangle (r, 5.0f);
    g.setColour (over ? Col::cyan : Col::line); g.drawRoundedRectangle (r, 5.0f, 1.0f);
    g.setColour (Col::cyan); g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText (juce::String (juce::CharPointer_UTF8 ("\xe2\x87\xa2 DRAG MIDI")), r, juce::Justification::centred);
}

juce::File DragMidi::make (const juce::File& dir)
{
    dir.createDirectory();
    const int bars = 1 << juce::jlimit (0, 3, (int) proc.apvts.getRawParameterValue ("arp_bars")->load());
    juce::String name = "Mister Arp - " + juce::String (marp::presets()[(size_t) juce::jlimit (0, (int) marp::presets().size() - 1, proc.getCurrentPreset())].name)
                      + " - " + juce::String (bars) + (bars == 1 ? " bar" : " bars");
    auto f = dir.getNonexistentChildFile (juce::File::createLegalFileName (name), ".mid", false);
    bool usedDefault = false;
    proc.exportMidi (bars, f, usedDefault);
    if (onStatus) onStatus (usedDefault ? "No chord held yet: exported with C minor. Hold a chord first to use yours."
                                        : "MIDI ready: " + juce::String (bars) + (bars == 1 ? " bar" : " bars") + " of the arp.");
    return f;
}

void DragMidi::mouseDrag (const juce::MouseEvent& e)
{
    if (dragged || e.getDistanceFromDragStart() < 5) return;
    dragged = true;
    auto f = make (juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("MisterArp"));
    juce::DragAndDropContainer::performExternalDragDropOfFiles ({ f.getFullPathName() }, false, this);
}

void DragMidi::mouseUp (const juce::MouseEvent&)
{
    if (dragged) return;
    auto f = make (juce::File::getSpecialLocation (juce::File::userMusicDirectory).getChildFile ("Mister Arp"));
    f.revealToUser();
    if (onStatus) onStatus ("Saved " + f.getFileName() + " in Music/Mister Arp.");
}

//============================================================================== Editor
MisterArpEditor::MisterArpEditor (MisterArpProcessor& p)
    : AudioProcessorEditor (&p), proc (p),
      presetBar (marp::presets()), scope (p.scope, Col::cyan),
      arpOn (p.apvts, "arp_on", "ARP ON", Col::cyan, "Arpeggiator on/off. Off = Mister Arp plays like a normal synth."),
      latch (p.apvts, "arp_latch", "LATCH", Col::amber, "Latch: the arp keeps playing after you let go. Play a new chord to replace it."),
      mode (p.apvts, "arp_mode", "Order of the notes: Up, Down, Up/Down, As Played, Random, Chord (all at once), Converge/Diverge, Pinky (alternates with the top note), Thumb (with the bottom note)."),
      rate (p.apvts, "arp_rate", "Step length: 1/16 = sixteenth notes, T = triplets, D = dotted."),
      key (p.apvts, "arp_key", "Key for the SCALE quantiser."),
      scale (p.apvts, "arp_scale", "Snaps every note (including per-step transposes) to a scale. Off = no snapping."),
      bars (p.apvts, "arp_bars", "How many bars DRAG MIDI exports."),
      grid (p), dragMidi (p), panel (p.apvts),
      keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);
    logo = juce::ImageCache::getFromMemory (BinaryData::TullyLogo_png, BinaryData::TullyLogo_pngSize);

    presetBar.getCurrent = [this] { return proc.getCurrentPreset(); };
    presetBar.load = [this] (int i) { proc.loadPreset (i); grid.repaint(); };
    presetBar.sync();

    auto k = [this] (const char* id, const char* cap, juce::Colour c, const char* tip)
    { knobs.push_back (std::make_unique<tui::Knob> (proc.apvts, id, cap, c, tip)); addAndMakeVisible (*knobs.back()); return knobs.back().get(); };
    oct = k ("arp_oct", "OCTAVES", Col::cyan, "How many octaves the arp climbs (1-4).");
    gate = k ("arp_gate", "GATE", Col::cyan, "Note length as a share of a step. Over 100% overlaps notes (slides in Legato mode).");
    swing = k ("arp_swing", "SWING", Col::cyan, "Pushes every second step later for a shuffle groove.");
    numSteps = k ("arp_steps", "STEPS", Col::cyan, "Pattern length (1-16). Odd lengths drift against the bar (polymeter).");
    velKeys = k ("arp_velkeys", "KEY VEL", Col::amber, "0% = step velocities only. 100% = how hard you play scales them too.");
    bpm = k ("arp_bpm", "TEMPO", Col::amber, "Tempo used when there is no host tempo (standalone app). In a DAW the arp follows the song.");
    quick.push_back (k ("cutoff", "CUTOFF", Col::pink, "Filter cutoff."));
    quick.push_back (k ("reso", "RESO", Col::pink, "Filter resonance."));
    quick.push_back (k ("flt_env", "ENV AMT", Col::pink, "Filter envelope amount: how plucky the notes are."));
    quick.push_back (k ("amp_d", "DECAY", Col::pink, "Note decay."));
    quick.push_back (k ("amp_r", "RELEASE", Col::pink, "Note release."));
    quick.push_back (k ("delay", "DELAY", Col::violet, "Ping-pong delay."));
    quick.push_back (k ("reverb", "REVERB", Col::violet, "Reverb."));
    quick.push_back (k ("volume", "VOLUME", Col::text, "Output volume."));

    arpTab.setClickingTogglesState (false); soundTab.setClickingTogglesState (false);
    arpTab.onClick = [this] { showPage (0); }; soundTab.onClick = [this] { showPage (1); };
    arpTab.setTooltip ("Arpeggiator: pattern, steps, scale and drag-out MIDI.");
    soundTab.setTooltip ("Every synth control: oscillators, filter, envelopes, LFO, FX.");
    for (auto* t : { &arpTab, &soundTab }) t->setColour (juce::TextButton::buttonOnColourId, Col::pink);

    status.setFont (juce::FontOptions (11.5f)); status.setColour (juce::Label::textColourId, Col::dim);
    status.setText ("Hold a chord to start the arp. Drag MIDI exports it to your DAW.", juce::dontSendNotification);
    dragMidi.onStatus = [this] (const juce::String& s) { status.setText (s, juce::dontSendNotification); };
    tempoLabel.setFont (juce::FontOptions (11.0f, juce::Font::bold)); tempoLabel.setColour (juce::Label::textColourId, Col::amber);
    tempoLabel.setJustificationType (juce::Justification::centredRight);

    keyboard.setAvailableRange (24, 108); keyboard.setLowestVisibleKey (24); keyboard.setScrollButtonsVisible (false);
    keyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour (0xffd9e1ea));
    keyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour (0xff13171d));
    keyboard.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour (0xff2a323d));
    keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, Col::cyan);
    keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, Col::pink.withAlpha (0.35f));
    keyboard.setColour (juce::MidiKeyboardComponent::shadowColourId, juce::Colours::transparentBlack);

    for (juce::Component* c : std::initializer_list<juce::Component*> { &presetBar, &scope, &arpTab, &soundTab, &arpOn, &latch, &mode, &rate, &key, &scale,
                                                                          &bars, &grid, &dragMidi, &status, &tempoLabel, &panel, &keyboard })
        addAndMakeVisible (c);

    setSize (1100, 700);
    showPage (0);
    startTimerHz (30);
}

MisterArpEditor::~MisterArpEditor() { stopTimer(); setLookAndFeel (nullptr); }

void MisterArpEditor::showPage (int p)
{
    page = p;
    arpTab.setToggleState (p == 0, juce::dontSendNotification);
    soundTab.setToggleState (p == 1, juce::dontSendNotification);
    const bool a = p == 0;
    for (juce::Component* c : std::initializer_list<juce::Component*> { &mode, &rate, &key, &scale, &grid }) c->setVisible (a);
    for (auto& kn : knobs) kn->setVisible (a);
    panel.setVisible (! a);
    repaint();
}

void MisterArpEditor::timerCallback()
{
    scope.refresh();
    presetBar.sync();
    const bool on = proc.apvts.getRawParameterValue ("arp_on")->load() > 0.5f;
    grid.setPlayingStep (on ? proc.arp.currentStep.load() : -1);
    const juce::String t = juce::String (juce::roundToInt (proc.tempo.load())) + " BPM" + (proc.hostTempo.load() ? "  (from your DAW)" : "  (TEMPO knob)");
    if (tempoLabel.getText() != t) tempoLabel.setText (t, juce::dontSendNotification);
    if (page == 0) grid.repaint();
}

void MisterArpEditor::paint (juce::Graphics& g)
{
    g.fillAll (Col::bg);
    tui::paintHeader (g, logo, "MISTER ARP", "advanced arpeggiator synth + drag-out MIDI", Col::cyan);
    if (page == 0)
    {
        tui::paintSection (g, rPattern, "PATTERN", Col::cyan);
        tui::paintSection (g, rScale, "SCALE + TEMPO", Col::amber);
        tui::paintSection (g, rQuick, "QUICK SOUND", Col::pink);
        tui::paintSection (g, rGrid, "STEPS", Col::pink);
    }
    tui::paintFooter (g, getLocalBounds().removeFromBottom (18).reduced (12, 0));
}

void MisterArpEditor::resized()
{
    auto b = getLocalBounds().reduced (12);
    b.removeFromBottom (12);
    auto header = b.removeFromTop (60);
    scope.setBounds (header.removeFromRight (190).reduced (0, 4));
    header.removeFromRight (12);
    presetBar.setBounds (header.removeFromRight (360).withTrimmedTop (2));
    b.removeFromTop (6);

    // tab row
    rTop = b.removeFromTop (28);
    {
        auto r = rTop;
        arpTab.setBounds (r.removeFromLeft (84)); r.removeFromLeft (4);
        soundTab.setBounds (r.removeFromLeft (84)); r.removeFromLeft (12);
        arpOn.setBounds (r.removeFromLeft (78)); r.removeFromLeft (4);
        latch.setBounds (r.removeFromLeft (70)); r.removeFromLeft (12);
        dragMidi.setBounds (r.removeFromRight (120)); r.removeFromRight (4);
        bars.setBounds (r.removeFromRight (82).reduced (0, 2)); r.removeFromRight (10);
        tempoLabel.setBounds (r.removeFromRight (190));
        status.setBounds (r);
    }
    b.removeFromTop (8);
    keyboard.setBounds (b.removeFromBottom (64));
    keyboard.setKeyWidth ((float) keyboard.getWidth() / 50.0f);
    b.removeFromBottom (8);

    panel.setBounds (b);

    // ARP page
    auto top = b.removeFromTop (128);
    b.removeFromTop (8);
    rGrid = b;
    rPattern = top.removeFromLeft (430); top.removeFromLeft (8);
    rScale = top.removeFromLeft (250); top.removeFromLeft (8);
    rQuick = top;
    {
        auto a = rPattern.reduced (8, 4); a.removeFromTop (24);
        auto combos = a.removeFromLeft (120);
        mode.setBounds (combos.removeFromTop (26).reduced (0, 1)); combos.removeFromTop (8);
        rate.setBounds (combos.removeFromTop (26).reduced (0, 1));
        a.removeFromLeft (8);
        const int w = a.getWidth() / 4;
        for (auto* kn : { oct, gate, swing, numSteps }) kn->setBounds (a.removeFromLeft (w));
    }
    {
        auto a = rScale.reduced (8, 4); a.removeFromTop (24);
        auto combos = a.removeFromLeft (110);
        key.setBounds (combos.removeFromTop (26).reduced (0, 1)); combos.removeFromTop (8);
        scale.setBounds (combos.removeFromTop (26).reduced (0, 1));
        a.removeFromLeft (6);
        const int w = a.getWidth() / 2;
        velKeys->setBounds (a.removeFromLeft (w)); bpm->setBounds (a);
    }
    {
        auto a = rQuick.reduced (6, 4); a.removeFromTop (24);
        const int w = a.getWidth() / (int) quick.size();
        for (auto* kn : quick) kn->setBounds (a.removeFromLeft (w));
    }
    grid.setBounds (rGrid.reduced (10, 8).withTrimmedTop (22));
}
