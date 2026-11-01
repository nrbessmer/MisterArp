// MATest: Mister Arp self-test.   MATest [outDir]
#include <juce_audio_utils/juce_audio_utils.h>
#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"
#include <chrono>

static int failures = 0;
static void check (bool ok, const juce::String& what, const juce::String& detail = {})
{
    printf ("  [%s] %-56s %s\n", ok ? "PASS" : "FAIL", what.toRawUTF8(), detail.toRawUTF8());
    if (! ok) ++failures;
}

// ---- a fake host playhead
struct Host : juce::AudioPlayHead
{
    double bpm = 120.0, ppq = 0.0; bool playing = true; bool hasTempo = true;
    juce::Optional<PositionInfo> getPosition() const override
    {
        PositionInfo p;
        if (hasTempo) p.setBpm (bpm);
        p.setPpqPosition (ppq); p.setIsPlaying (playing);
        return p;
    }
};

static std::vector<int> orderFor (int mode, std::vector<int> notes, int octaves = 1, int steps = 12, std::vector<int> playOrder = {})
{
    marp::Settings s; s.mode = mode; s.octaves = octaves;
    std::vector<marp::HeldNote> h;
    for (size_t i = 0; i < notes.size(); ++i) h.push_back ({ notes[i], 0.8f, playOrder.empty() ? (int) i : playOrder[i] });
    marp::Generator g; g.reset(); g.setNotes (h, s);
    std::vector<int> out; std::vector<marp::OutNote> tmp;
    for (int k = 0; k < steps; ++k) { tmp.clear(); g.generate (k, s, tmp); for (auto& n : tmp) out.push_back (n.note); }
    return out;
}
static juce::String str (const std::vector<int>& v) { juce::StringArray s; for (int x : v) s.add (juce::String (x)); return s.joinIntoString (" "); }

// run the processor with MIDI key events; returns the notes the arp sent (via a tap on the synth input)
struct Tap { std::vector<std::pair<juce::int64, juce::MidiMessage>> ev; };

static juce::AudioBuffer<float> runProc (MisterArpProcessor& p, Host& host, const std::vector<std::pair<double, juce::MidiMessage>>& keys,
                                         double seconds, double sr, int block, bool advanceHost = true)
{
    const int N = (int) (seconds * sr);
    juce::AudioBuffer<float> out (2, N); out.clear();
    juce::AudioBuffer<float> buf (2, block);
    size_t e = 0;
    for (int pos = 0; pos < N; pos += block)
    {
        const int n = juce::jmin (block, N - pos);
        buf.setSize (2, n, false, false, true); buf.clear();
        juce::MidiBuffer mb;
        while (e < keys.size() && (int) (keys[e].first * sr) < pos + n) { mb.addEvent (keys[e].second, juce::jmax (0, (int) (keys[e].first * sr) - pos)); ++e; }
        p.processBlock (buf, mb);
        for (int c = 0; c < 2; ++c) out.copyFrom (c, pos, buf, c, 0, n);
        if (advanceHost && host.playing) host.ppq += n * host.bpm / 60.0 / sr;
    }
    return out;
}

// drive the Player directly and collect timestamped note events
struct Collected { std::vector<std::tuple<juce::int64, int, bool>> ev; int ons = 0, offs = 0; };
static Collected runPlayer (marp::Player& pl, const marp::Settings& s, const std::vector<std::pair<juce::int64, juce::MidiMessage>>& keys,
                            juce::int64 total, double sr, int block, double bpm, bool hostPlaying, bool latch = false,
                            std::function<double (juce::int64)> ppqAt = {})
{
    Collected c; size_t e = 0;
    for (juce::int64 pos = 0; pos < total; pos += block)
    {
        const int n = (int) juce::jmin ((juce::int64) block, total - pos);
        juce::MidiBuffer in, out;
        while (e < keys.size() && keys[e].first < pos + n) { in.addEvent (keys[e].second, (int) (keys[e].first - pos)); ++e; }
        const double ppq = ppqAt ? ppqAt (pos) : (double) pos * bpm / 60.0 / sr;
        pl.process (in, out, n, s, latch, hostPlaying, ppq, bpm);
        for (const auto m : out)
        {
            const auto msg = m.getMessage();
            if (msg.isNoteOn()) { c.ev.push_back ({ pos + m.samplePosition, msg.getNoteNumber(), true }); ++c.ons; }
            else if (msg.isNoteOff()) { c.ev.push_back ({ pos + m.samplePosition, msg.getNoteNumber(), false }); ++c.offs; }
        }
    }
    return c;
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    const juce::File outDir (argc > 1 ? argv[1] : "/tmp/matest");
    outDir.createDirectory();
    const double sr = 48000.0;

    printf ("Modes (C E G)\n");
    {
        const std::vector<int> ceg { 60, 64, 67 };
        struct T { int mode; const char* name; std::vector<int> want; };
        const std::vector<T> tests = {
            { marp::Up, "Up", { 60, 64, 67, 60, 64, 67 } },
            { marp::Down, "Down", { 67, 64, 60, 67, 64, 60 } },
            { marp::UpDown, "Up/Down", { 60, 64, 67, 64, 60, 64 } },
            { marp::DownUp, "Down/Up", { 67, 64, 60, 64, 67, 64 } },
            { marp::Converge, "Converge", { 60, 67, 64, 60, 67, 64 } },
            { marp::Diverge, "Diverge", { 64, 67, 60, 64, 67, 60 } },
            { marp::PinkyUp, "Pinky", { 60, 67, 64, 67, 60, 67 } },
            { marp::ThumbUp, "Thumb", { 60, 64, 60, 67, 60, 64 } } };
        for (auto& t : tests)
        {
            auto got = orderFor (t.mode, ceg, 1, 6);
            check (got == t.want, juce::String ("mode ") + t.name, str (got));
        }
        auto ap = orderFor (marp::AsPlayed, { 60, 64, 67 }, 1, 6, { 2, 0, 1 });   // played E, G, C
        check (ap == std::vector<int> { 64, 67, 60, 64, 67, 60 }, "mode As Played (E, G, C)", str (ap));
        auto up2 = orderFor (marp::Up, ceg, 2, 6);
        check (up2 == std::vector<int> { 60, 64, 67, 72, 76, 79 }, "Up over 2 octaves", str (up2));
        auto ch = orderFor (marp::Chord, ceg, 1, 2);
        check (ch == std::vector<int> { 60, 64, 67, 60, 64, 67 }, "Chord plays all notes each step", str (ch));
        auto rnd = orderFor (marp::Random, ceg, 2, 200);
        std::set<int> distinct (rnd.begin(), rnd.end());
        check (distinct.size() == 6, "Random uses every note of the range", juce::String ((int) distinct.size()) + " distinct");
    }

    printf ("\nSteps\n");
    {
        marp::Settings s; s.mode = marp::Up; s.stepBeats = 0.25;
        std::vector<marp::HeldNote> h { { 60, 0.8f, 0 } };
        auto gen = [&] (int steps) { marp::Generator g; g.reset(); g.setNotes (h, s); std::vector<marp::OutNote> o; for (int k = 0; k < steps; ++k) g.generate (k, s, o); return o; };

        s.steps[1].ratchet = 3;
        auto o = gen (2);
        check (o.size() == 4 && std::abs (o[2].beat - (0.25 + 0.25 / 3)) < 1e-9, "ratchet x3 = 3 evenly spaced notes", juce::String ((int) o.size()) + " notes");
        s.steps[1].ratchet = 1;

        s.steps[1].prob = 0.0f; o = gen (2);
        check (o.size() == 1, "chance 0% never plays", juce::String ((int) o.size()));
        s.steps[1].prob = 0.5f; o = gen (32);   // 16 x step 1 (k=1,17) only... use many steps
        { marp::Settings s2 = s; for (auto& st : s2.steps) st.prob = 0.5f; marp::Generator g; g.reset(); g.setNotes (h, s2); std::vector<marp::OutNote> oo;
          for (int k = 0; k < 2000; ++k) g.generate (k, s2, oo);
          check (oo.size() > 900 && oo.size() < 1100, "chance 50% plays about half", juce::String ((int) oo.size()) + "/2000"); }
        s.steps[1].prob = 1.0f;

        s.steps[1].tie = true; s.steps[2].tie = true; s.gate = 0.5f; o = gen (4);
        check (o.size() == 2 && std::abs (o[0].len - (2 * 0.25 + 0.5 * 0.25)) < 1e-9, "two ties stretch the note over 3 steps", juce::String (o[0].len, 3) + " beats");
        s.steps[1].tie = s.steps[2].tie = false;

        s.swing = 0.5f; o = gen (2);
        check (std::abs (o[1].beat - (0.25 + 0.5 * 0.5 * 0.25)) < 1e-9, "swing 50% delays odd steps by 1/4 step", juce::String (o[1].beat, 4));
        s.swing = 0.0f;

        s.steps[0].trans = 4; s.scale = 2; s.key = 0; o = gen (1);   // C minor: E -> Eb
        check (o[0].note == 63, "scale quantise (C minor): C+4 -> Eb", juce::String (o[0].note));
        s.steps[0].trans = 0; s.scale = 0;

        s.numSteps = 5; s.steps[4].on = false; o = gen (10);
        check (o.size() == 8, "5-step pattern with a rest repeats every 5 steps", juce::String ((int) o.size()) + " of 10");
    }

    printf ("\nTiming (host at 120 BPM, 1/16 = 6000 samples)\n");
    for (int blk : { 37, 512, 1024 })
    {
        marp::Player pl; pl.prepare (sr);
        marp::Settings s; s.stepBeats = 0.25; s.gate = 0.5f;
        auto c = runPlayer (pl, s, { { 0, juce::MidiMessage::noteOn (1, 60, 0.8f) }, { 0, juce::MidiMessage::noteOn (1, 64, 0.8f) } },
                            (juce::int64) (sr * 2), sr, blk, 120.0, true);
        int bad = 0, ons = 0;
        for (auto& [t, n, on] : c.ev) if (on) { if (std::abs ((double) t - std::round ((double) t / 6000.0) * 6000.0) > 1.0) ++bad; ++ons; }
        check (bad == 0 && ons == 16, "block " + juce::String (blk) + ": every note on the 16th grid", juce::String (ons) + " notes, " + juce::String (bad) + " off-grid");
    }
    {
        // free running (no host transport): starts at once, keeps time
        marp::Player pl; pl.prepare (sr);
        marp::Settings s; s.stepBeats = 0.25;
        auto c = runPlayer (pl, s, { { 1000, juce::MidiMessage::noteOn (1, 60, 0.8f) } }, (juce::int64) (sr * 1), sr, 512, 120.0, false);
        const auto first = c.ev.empty() ? (juce::int64) -1 : std::get<0> (c.ev.front());
        check (first == 1000 && c.ons == 8, "free run: first note exactly on the key press", "first at " + juce::String (first) + ", " + juce::String (c.ons) + " notes");
    }
    {
        // release -> all notes end; latch keeps playing; new chord replaces latch
        marp::Player pl; pl.prepare (sr);
        marp::Settings s; s.stepBeats = 0.25; s.gate = 0.9f;
        auto c = runPlayer (pl, s, { { 0, juce::MidiMessage::noteOn (1, 60, 0.8f) }, { 24000, juce::MidiMessage::noteOff (1, 60) } },
                            (juce::int64) (sr * 1.5), sr, 256, 120.0, true);
        check (c.ons == c.offs && c.ons == 4, "release: no stuck notes", juce::String (c.ons) + " on / " + juce::String (c.offs) + " off");
        marp::Player pl2; pl2.prepare (sr);
        auto d = runPlayer (pl2, s, { { 0, juce::MidiMessage::noteOn (1, 60, 0.8f) }, { 24000, juce::MidiMessage::noteOff (1, 60) },
                                      { 48000, juce::MidiMessage::noteOn (1, 67, 0.8f) }, { 50000, juce::MidiMessage::noteOff (1, 67) } },
                            (juce::int64) (sr * 2), sr, 256, 120.0, true, true);
        int c60after = 0, c67 = 0;
        for (auto& [t, n, on] : d.ev) if (on) { if (n == 60 && t > 48100) ++c60after; if (n == 67) ++c67; }
        check (c60after == 0 && c67 >= 7, "latch: keeps playing, new chord replaces it", juce::String (c67) + " x G after the new chord");
    }
    {
        // host loop: ppq jumps back every bar -> still balanced, still on grid
        marp::Player pl; pl.prepare (sr);
        marp::Settings s; s.stepBeats = 0.25; s.gate = 0.95f;
        const juce::int64 bar = (juce::int64) (sr * 2);   // 4 beats at 120
        auto c = runPlayer (pl, s, { { 0, juce::MidiMessage::noteOn (1, 60, 0.8f) } }, bar * 4, sr, 512, 120.0, true, false,
                            [bar, sr] (juce::int64 pos) { return (double) (pos % bar) * 2.0 / sr; });
        check (std::abs (c.ons - 64) <= 1 && c.ons - c.offs <= 1, "host loop: 16 steps per bar, no stuck notes", juce::String (c.ons) + " on / " + juce::String (c.offs) + " off");
    }

    printf ("\nMIDI export\n");
    {
        marp::Settings s; s.stepBeats = 0.25; s.mode = marp::Up; s.gate = 0.5f;
        auto f = juce::File (outDir.getChildFile ("export.mid"));
        marp::exportMidi ({ { 60, 0.8f, 0 }, { 64, 0.8f, 1 }, { 67, 0.8f, 2 } }, s, 4, 128.0, f);
        juce::MidiFile mf; juce::FileInputStream is (f); const bool ok = mf.readFrom (is);
        int ons = 0; double lastOff = 0; bool order = true; int prev = -1; std::vector<int> first;
        if (ok && mf.getNumTracks() >= 2)
            for (auto* e : *mf.getTrack (1))
            {
                if (e->message.isNoteOn()) { ++ons; if (first.size() < 6) first.push_back (e->message.getNoteNumber()); if (e->message.getTimeStamp() < prev) order = false; prev = (int) e->message.getTimeStamp(); }
                if (e->message.isNoteOff()) lastOff = juce::jmax (lastOff, e->message.getTimeStamp());
            }
        check (ok && ons == 64 && order, "4 bars of 16ths = 64 notes, in order", juce::String (ons) + " notes");
        check (first == std::vector<int> { 60, 64, 67, 60, 64, 67 }, "exported notes follow the arp", str (first));
        check (lastOff <= 4 * 4 * 960, "nothing runs past the last bar", juce::String (lastOff / 960.0, 2) + " beats");
    }

    printf ("\nPresets\n");
    MisterArpProcessor p;
    Host host;
    p.setPlayHead (&host);
    p.setPlayConfigDetails (0, 2, sr, 512);
    juce::String table; int bad = 0;
    std::vector<std::pair<double, juce::MidiMessage>> chord = {
        { 0.02, juce::MidiMessage::noteOn (1, 57, 0.8f) }, { 0.02, juce::MidiMessage::noteOn (1, 60, 0.8f) }, { 0.02, juce::MidiMessage::noteOn (1, 64, 0.8f) },
        { 4.0, juce::MidiMessage::noteOff (1, 57) }, { 4.0, juce::MidiMessage::noteOff (1, 60) }, { 4.0, juce::MidiMessage::noteOff (1, 64) } };
    for (int i = 0; i < (int) marp::presets().size(); ++i)
    {
        p.loadPreset (i); p.prepareToPlay (sr, 512); host.ppq = 0; host.playing = true;
        auto a = runProc (p, host, chord, 6.0, sr, 512);
        float pk = 0; double acc = 0; int nans = 0;
        for (int c = 0; c < 2; ++c) for (int k = 0; k < a.getNumSamples(); ++k) { const float x = a.getSample (c, k); if (! std::isfinite (x)) { ++nans; continue; } pk = juce::jmax (pk, std::abs (x)); acc += (double) x * x; }
        const float rms = (float) (10.0 * std::log10 (acc / (2.0 * 4.0 * sr) + 1e-12));   // over the 4 s the chord is held
        // the tail after release must die away (no stuck notes)
        float tail = 0; for (int c = 0; c < 2; ++c) tail = juce::jmax (tail, a.getMagnitude (c, (int) (5.6 * sr), (int) (0.4 * sr)));
        const bool tailOk = tail < 0.05f || p.apvts.getRawParameterValue ("reverb")->load() > 0.4f || p.apvts.getRawParameterValue ("delay_fb")->load() > 0.45f;
        const bool ok = nans == 0 && pk < 1.0f && rms > -36.0f && rms < -8.0f && tailOk;
        if (! ok) ++bad;
        const auto& pr = marp::presets()[(size_t) i];
        table << juce::String (i).paddedLeft ('0', 2) << " " << juce::String (pr.name).paddedRight (' ', 22) << " peak " << juce::String (pk, 3)
              << " rms " << juce::String (rms, 1) << " dB" << " tail " << juce::String (tail, 3) << (ok ? "" : "  <-- check") << "\n";
    }
    outDir.getChildFile ("preset_levels.txt").replaceWithText (table);
    printf ("%s", table.toRawUTF8());
    check (bad == 0, "every preset renders clean and stops after release", juce::String (bad) + " to check");
    check (marp::presets().size() >= 45, "preset count", juce::String ((int) marp::presets().size()));

    printf ("\nPlugin\n");
    {
        // arp off = plain synth
        p.loadPreset (0); auto* on = p.apvts.getParameter ("arp_on"); on->setValueNotifyingHost (0.0f);
        p.prepareToPlay (sr, 512); host.ppq = 0;
        runProc (p, host, { { 0.01, juce::MidiMessage::noteOn (1, 60, 0.8f) }, { 0.01, juce::MidiMessage::noteOn (1, 64, 0.8f) } }, 0.5, sr, 512);
        check (p.engine.activeVoices() == 2, "ARP OFF: plays held notes like a normal synth", juce::String (p.engine.activeVoices()) + " voices");
        on->setValueNotifyingHost (1.0f);

        // standalone (no host tempo): uses the TEMPO knob
        host.hasTempo = false; host.playing = false;
        p.prepareToPlay (sr, 512);
        runProc (p, host, { { 0.0, juce::MidiMessage::noteOn (1, 60, 0.8f) } }, 0.3, sr, 512, false);
        check (std::abs (p.tempo.load() - 128.0) < 0.01 && ! p.hostTempo.load(), "no host tempo -> TEMPO knob (128 BPM)", juce::String (p.tempo.load()));
        host.hasTempo = true; host.playing = true;

        // pattern code + state round trip
        p.loadPreset (8);
        const auto code = p.patternToCode();
        juce::MemoryBlock mb; p.getStateInformation (mb);
        MisterArpProcessor q; q.setStateInformation (mb.getData(), (int) mb.getSize());
        int diff = 0;
        for (auto* prm : p.getParameters())
            if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (prm))
                if (std::abs (rp->getValue() - q.apvts.getParameter (rp->getParameterID())->getValue()) > 1e-5f) ++diff;
        check (diff == 0 && q.patternToCode() == code && q.getCurrentPreset() == 8, "state save / restore incl. steps", code);
        q.setPatternFromCode (code);
        check (q.patternToCode() == code, "pattern code round trip", "");

        // CPU: 3-octave arp + 8 voices of supersaw
        p.loadPreset (1); p.prepareToPlay (sr, 512); host.ppq = 0;
        const auto t0 = std::chrono::steady_clock::now();
        runProc (p, host, chord, 10.0, sr, 512);
        const double secs = std::chrono::duration<double> (std::chrono::steady_clock::now() - t0).count();
        check (secs < 10.0 * 0.35, "CPU: supersaw arp, 10 s", juce::String (secs * 10.0, 1) + "% of one core");
    }

    printf ("\nEditor\n");
    {
        p.loadPreset (0);
        std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
        auto* e = static_cast<MisterArpEditor*> (ed.get());
        e->timerCallback();
        for (int pg : { 0, 1 })
        {
            e->showPage (pg);
            auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f);
            juce::File f = outDir.getChildFile (pg == 0 ? "ui_arp.png" : "ui_sound.png"); f.deleteFile();
            juce::FileOutputStream os (f); juce::PNGImageFormat().writeImageToStream (img, os);
            check (img.isValid(), juce::String ("editor page ") + (pg == 0 ? "ARP" : "SOUND") + " draws", f.getFileName());
        }
    }
    p.setPlayHead (nullptr);
    printf ("\n%s  (%d failures)\n", failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED", failures);
    return failures == 0 ? 0 : 1;
}
