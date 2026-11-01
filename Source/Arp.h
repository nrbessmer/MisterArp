/*
  ==============================================================================
    Mister Arp - Arp.h
    (c)(p) 2026 Tully EDM Vibe

    Arpeggiator: 11 modes, 1-4 octaves, note values 1/32..1/4 (dotted, triplet),
    swing, gate, latch, a 16-step pattern (on/off, velocity, ratchet 1-4,
    probability, transpose, tie) and scale quantising. The same step generator
    drives live playback and the MIDI export, so the dragged MIDI matches what
    you hear (random modes and probability use a fixed seed for the export).
  ==============================================================================
*/
#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "SynthParams.h"
#include <vector>
#include <array>
#include <algorithm>

namespace marp
{
    static constexpr int kMaxSteps = 16;

    enum Mode { Up = 0, Down, UpDown, DownUp, AsPlayed, Random, Chord, Converge, Diverge, PinkyUp, ThumbUp, kNumModes };
    inline const juce::StringArray& modeNames()
    {
        static const juce::StringArray n { "Up", "Down", "Up/Down", "Down/Up", "As Played", "Random", "Chord", "Converge", "Diverge", "Pinky", "Thumb" };
        return n;
    }

    // arp rate choices: index into tsyn::noteValues() (1/32 .. 1/4)
    inline const std::vector<int>& rateIndices() { static const std::vector<int> r { 0, 1, 2, 3, 4, 5, 6, 7, 8 }; return r; }
    inline juce::StringArray rateNames() { juce::StringArray s; for (int i : rateIndices()) s.add (tsyn::noteValues()[(size_t) i].name); return s; }
    inline double rateBeats (int choice) { return tsyn::noteValues()[(size_t) rateIndices()[(size_t) juce::jlimit (0, (int) rateIndices().size() - 1, choice)]].beats; }

    struct Scale { const char* name; std::vector<int> steps; };
    inline const std::vector<Scale>& scales()
    {
        static const std::vector<Scale> s = {
            { "Off", {} },
            { "Major", { 0, 2, 4, 5, 7, 9, 11 } }, { "Minor", { 0, 2, 3, 5, 7, 8, 10 } },
            { "Dorian", { 0, 2, 3, 5, 7, 9, 10 } }, { "Phrygian", { 0, 1, 3, 5, 7, 8, 10 } },
            { "Lydian", { 0, 2, 4, 6, 7, 9, 11 } }, { "Mixolydian", { 0, 2, 4, 5, 7, 9, 10 } },
            { "Harmonic Minor", { 0, 2, 3, 5, 7, 8, 11 } }, { "Minor Pentatonic", { 0, 3, 5, 7, 10 } },
            { "Major Pentatonic", { 0, 2, 4, 7, 9 } }, { "Blues", { 0, 3, 5, 6, 7, 10 } } };
        return s;
    }
    inline juce::StringArray scaleNames() { juce::StringArray s; for (auto& x : scales()) s.add (x.name); return s; }
    inline juce::StringArray keyNames() { return { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }; }

    // nearest note in the scale (ties go down)
    inline int quantise (int note, int key, int scale)
    {
        if (scale <= 0 || scale >= (int) scales().size()) return note;
        const auto& st = scales()[(size_t) scale].steps;
        for (int d = 0; d <= 6; ++d)
            for (int sgn : { -1, 1 })
            {
                const int c = note + sgn * d;
                const int pc = ((c - key) % 12 + 12) % 12;
                if (std::find (st.begin(), st.end(), pc) != st.end()) return c;
            }
        return note;
    }

    struct Step
    {
        bool  on = true;
        float vel = 0.8f;      // 0..1
        int   ratchet = 1;     // 1..4
        float prob = 1.0f;     // 0..1
        int   trans = 0;       // -12..12 semitones
        bool  tie = false;     // hold the previous note through this step
    };

    struct Settings
    {
        int    mode = Up;
        double stepBeats = 0.25;
        int    octaves = 1;
        float  gate = 0.6f;       // fraction of a step (up to 1.5 = overlapping)
        float  swing = 0.0f;      // 0..1 -> every 2nd step later by up to half a step
        int    numSteps = 16;
        int    key = 0, scale = 0;
        float  velFromKeys = 0.0f;  // 0 = step velocity only, 1 = played velocity scales it
        std::array<Step, kMaxSteps> steps {};
    };

    struct HeldNote { int note; float vel; int order; };
    struct OutNote  { double beat; double len; int note; float vel; };

    //==========================================================================
    // Deterministic step generator. Call generate(k) for k = 0,1,2,... in order.
    class Generator
    {
    public:
        void reset (uint32_t seed = 0x5eed) noexcept { noteIndex = 0; rng = juce::Random ((juce::int64) seed); }

        // builds the note order for the current held notes and settings
        void setNotes (const std::vector<HeldNote>& held, const Settings& s)
        {
            seq.clear(); chord.clear();
            if (held.empty()) return;
            std::vector<HeldNote> sorted = held;
            std::sort (sorted.begin(), sorted.end(), [] (auto& a, auto& b) { return a.note < b.note; });
            std::vector<HeldNote> played = held;
            std::sort (played.begin(), played.end(), [] (auto& a, auto& b) { return a.order < b.order; });
            for (auto& h : sorted) chord.push_back ({ h.note, h.vel, 0 });

            std::vector<HeldNote> asc;
            for (int o = 0; o < juce::jlimit (1, 4, s.octaves); ++o)
                for (auto& h : (s.mode == AsPlayed ? played : sorted)) asc.push_back ({ h.note + 12 * o, h.vel, 0 });

            const int n = (int) asc.size();
            switch (s.mode)
            {
                case Down: seq.assign (asc.rbegin(), asc.rend()); break;
                case UpDown:
                    seq = asc;
                    for (int i = n - 2; i >= 1; --i) seq.push_back (asc[(size_t) i]);
                    break;
                case DownUp:
                    seq.assign (asc.rbegin(), asc.rend());
                    for (int i = 1; i <= n - 2; ++i) seq.push_back (asc[(size_t) i]);
                    break;
                case Converge:
                    for (int lo = 0, hi = n - 1; lo <= hi; ++lo, --hi) { seq.push_back (asc[(size_t) lo]); if (hi != lo) seq.push_back (asc[(size_t) hi]); }
                    break;
                case Diverge:
                {
                    std::vector<HeldNote> c;
                    for (int lo = 0, hi = n - 1; lo <= hi; ++lo, --hi) { c.push_back (asc[(size_t) lo]); if (hi != lo) c.push_back (asc[(size_t) hi]); }
                    seq.assign (c.rbegin(), c.rend());
                    break;
                }
                case PinkyUp:
                    if (n == 1) seq = asc;
                    else for (int i = 0; i < n - 1; ++i) { seq.push_back (asc[(size_t) i]); seq.push_back (asc.back()); }
                    break;
                case ThumbUp:
                    if (n == 1) seq = asc;
                    else for (int i = 1; i < n; ++i) { seq.push_back (asc.front()); seq.push_back (asc[(size_t) i]); }
                    break;
                case Up: case AsPlayed: case Random: case Chord: default: seq = asc; break;
            }
            if (noteIndex >= (int) seq.size()) noteIndex = 0;
        }

        bool hasNotes() const noexcept { return ! seq.empty(); }

        // step k on the grid -> zero or more notes (appended to out). Returns the pattern step index.
        int generate (juce::int64 k, const Settings& s, std::vector<OutNote>& out)
        {
            const int ns = juce::jlimit (1, kMaxSteps, s.numSteps);
            const int si = (int) (k % ns);
            const auto& st = s.steps[(size_t) si];
            if (seq.empty() || ! st.on || st.tie) return si;
            if (st.prob < 0.999f && rng.nextFloat() >= st.prob) return si;

            // ties on the following steps extend this note
            int ties = 0;
            for (int j = 1; j < ns; ++j)
            {
                const auto& nx = s.steps[(size_t) ((si + j) % ns)];
                if (nx.on && nx.tie) ++ties; else break;
            }
            const double sb = s.stepBeats;
            const double start = stepStart (k, s);
            const int r = juce::jlimit (1, 4, st.ratchet);
            const double sub = sb / r;
            const double len = ties > 0 ? (double) ties * sb + s.gate * sb : juce::jmax (0.01, s.gate * sub);

            auto addNote = [&] (const HeldNote& h, double at, double l, float velMul)
            {
                int note = h.note + st.trans;
                note = quantise (note, s.key, s.scale);
                while (note > 120) note -= 12;
                while (note < 12) note += 12;
                const float v = juce::jlimit (0.05f, 1.0f, st.vel * velMul * (1.0f - s.velFromKeys + s.velFromKeys * h.vel));
                out.push_back ({ at, l, note, v });
            };

            if (s.mode == Chord)
            {
                const int oct = (int) (counter % juce::jlimit (1, 4, s.octaves));
                for (int j = 0; j < r; ++j)
                    for (auto h : chord) { h.note += 12 * oct; addNote (h, start + j * sub, r > 1 ? juce::jmax (0.01, s.gate * sub) : len, j == 0 ? 1.0f : 0.8f); }
                ++counter;
                return si;
            }
            const HeldNote& h = s.mode == Random ? seq[(size_t) rng.nextInt ((int) seq.size())] : seq[(size_t) (noteIndex % (int) seq.size())];
            for (int j = 0; j < r; ++j) addNote (h, start + j * sub, r > 1 ? juce::jmax (0.01, s.gate * sub) : len, j == 0 ? 1.0f : 0.85f);
            noteIndex = (noteIndex + 1) % (int) seq.size();
            ++counter;
            return si;
        }

        static double stepStart (juce::int64 k, const Settings& s) noexcept
        {
            const double base = (double) k * s.stepBeats;
            return (k % 2 == 1) ? base + s.swing * 0.5 * s.stepBeats : base;
        }

    private:
        std::vector<HeldNote> seq, chord;
        int noteIndex = 0;
        juce::int64 counter = 0;
        juce::Random rng { 0x5eed };
    };

    //==========================================================================
    // Real-time arp: turns held keys into timed note on/offs in a MidiBuffer.
    class Player
    {
    public:
        void prepare (double sampleRate) { sr = sampleRate; reset(); }
        void reset()
        {
            held.clear(); latched.clear(); pending.clear(); gen.reset ((uint32_t) juce::Time::getMillisecondCounter());
            freeBeat = 0.0; lastStep = -1; nextK = -1; orderCounter = 0; anyPhysical = false; currentStep.store (-1);
        }

        // keys in, notes out (into `out`, at sample offsets). The caller passes non-note MIDI through itself.
        void process (const juce::MidiBuffer& in, juce::MidiBuffer& out, int numSamples, const Settings& s, bool latch,
                      bool hostPlaying, double hostPpq, double bpm)
        {
            const double beatsPerSample = juce::jlimit (30.0, 300.0, bpm) / 60.0 / sr;
            double beat0;
            if (hostPlaying) { beat0 = hostPpq; if (! wasHostPlaying) { flushAll (out, 0); nextK = -1; } }
            else beat0 = freeBeat;
            if (wasHostPlaying && ! hostPlaying) { flushAll (out, 0); nextK = -1; }
            if (hostPlaying && wasHostPlaying && std::abs (beat0 - expectBeat) > 1.0e-3) { flushAll (out, 0); nextK = -1; }   // loop / jump
            wasHostPlaying = hostPlaying;

            // ---- keys
            bool changed = false;
            int firstOnAt = -1;
            for (const auto m : in)
            {
                const auto msg = m.getMessage();
                if (msg.isNoteOn())
                {
                    if (firstOnAt < 0) firstOnAt = m.samplePosition;
                    if (latch && ! anyPhysical) latched.clear();   // new chord replaces the latched one
                    anyPhysical = true;
                    removeFrom (held, msg.getNoteNumber());
                    held.push_back ({ msg.getNoteNumber(), msg.getFloatVelocity(), orderCounter++ });
                    if (latch) { removeFrom (latched, msg.getNoteNumber()); latched.push_back (held.back()); }
                    changed = true;
                }
                else if (msg.isNoteOff())
                {
                    removeFrom (held, msg.getNoteNumber());
                    anyPhysical = ! held.empty();
                    changed = true;
                }
                else if (msg.isAllNotesOff() || msg.isAllSoundOff()) { held.clear(); latched.clear(); anyPhysical = false; changed = true; }
            }
            if (! latch && ! latched.empty()) { latched.clear(); changed = true; }
            const auto& active = latch ? latched : held;
            if (changed || lastLatch != latch || lastMode != s.mode || lastOct != s.octaves)
            {
                const bool wasEmpty = ! gen.hasNotes();
                gen.setNotes (active, s);
                if (wasEmpty && gen.hasNotes() && ! hostPlaying)   // free run: step 0 lands exactly on the key press
                { beat0 = -juce::jmax (0, firstOnAt) * beatsPerSample; freeBeat = beat0; nextK = -1; }
                lastLatch = latch; lastMode = s.mode; lastOct = s.octaves;
            }
            if (active.empty() && gen.hasNotes()) gen.setNotes (active, s);

            const double beat1 = beat0 + numSamples * beatsPerSample;
            auto toSample = [&] (double b) { return juce::jlimit (0, numSamples - 1, (int) std::floor ((b - beat0) / beatsPerSample)); };

            // ---- note-offs due in this block
            for (auto it = pending.begin(); it != pending.end();)
            {
                if (it->beat < beat1) { out.addEvent (juce::MidiMessage::noteOff (1, it->note), toSample (it->beat)); it = pending.erase (it); }
                else ++it;
            }

            // ---- steps starting in this block
            if (gen.hasNotes())
            {
                juce::int64 k = juce::jmax ((juce::int64) 0, (juce::int64) std::floor (beat0 / s.stepBeats) - 1);
                if (nextK >= 0 && nextK > k && nextK - k < 4) k = nextK;   // continue without re-firing a step
                for (; ; ++k)
                {
                    const double st = Generator::stepStart (k, s);
                    if (st >= beat1) break;
                    if (st < beat0 - 1.0e-9) continue;
                    notes.clear();
                    const int si = gen.generate (k, s, notes);
                    currentStep.store (si);
                    for (auto& nn : notes)
                    {
                        // retrigger: end a still-sounding copy of the same note first
                        for (auto it = pending.begin(); it != pending.end();)
                            if (it->note == nn.note) { out.addEvent (juce::MidiMessage::noteOff (1, nn.note), toSample (nn.beat)); it = pending.erase (it); } else ++it;
                        out.addEvent (juce::MidiMessage::noteOn (1, nn.note, nn.vel), toSample (nn.beat));
                        if (nn.beat + nn.len < beat1) out.addEvent (juce::MidiMessage::noteOff (1, nn.note), juce::jmin (numSamples - 1, toSample (nn.beat + nn.len) + 1));
                        else pending.push_back ({ nn.beat + nn.len, 0.0, nn.note, 0.0f });
                    }
                    nextK = k + 1;
                }
            }
            else currentStep.store (-1);

            freeBeat = beat1; expectBeat = beat1;
            if (pending.size() > 64) pending.erase (pending.begin(), pending.begin() + 32);
        }

        void flushAll (juce::MidiBuffer& out, int at)
        {
            for (auto& p : pending) out.addEvent (juce::MidiMessage::noteOff (1, p.note), at);
            pending.clear();
        }

        // notes the export should use: latched/held, else the last chord played
        std::vector<HeldNote> notesForExport (bool latch) const
        {
            const auto& a = latch ? latched : held;
            if (! a.empty()) return a;
            return lastChord;
        }
        void rememberChord() { const auto& a = ! latched.empty() ? latched : held; if (! a.empty()) lastChord = a; }

        std::atomic<int> currentStep { -1 };

    private:
        static void removeFrom (std::vector<HeldNote>& v, int note)
        {
            v.erase (std::remove_if (v.begin(), v.end(), [note] (auto& h) { return h.note == note; }), v.end());
        }
        double sr = 44100.0;
        Generator gen;
        std::vector<HeldNote> held, latched, lastChord;
        std::vector<OutNote> pending, notes;
        double freeBeat = 0.0, expectBeat = 0.0;
        juce::int64 lastStep = -1, nextK = -1;
        int orderCounter = 0;
        bool anyPhysical = false, wasHostPlaying = false, lastLatch = false;
        int lastMode = -1, lastOct = -1;
    };

    //==========================================================================
    // Offline render of `bars` bars to a Standard MIDI File (960 ppq).
    inline juce::File exportMidi (const std::vector<HeldNote>& notes, const Settings& s, int bars, double bpm, const juce::File& file)
    {
        constexpr int tpq = 960;
        Generator g; g.reset (0x5eed); g.setNotes (notes, s);
        std::vector<OutNote> out, tmp;
        const double total = bars * 4.0;
        for (juce::int64 k = 0; ; ++k)
        {
            if (Generator::stepStart (k, s) >= total) break;
            tmp.clear(); g.generate (k, s, tmp);
            for (auto& n : tmp) if (n.beat < total) { n.len = juce::jmin (n.len, total - n.beat); out.push_back (n); }
        }
        juce::MidiMessageSequence seq;
        seq.addEvent (juce::MidiMessage::textMetaEvent (3, "Mister Arp"), 0);
        // retriggers of the same pitch: shorten the earlier note so offs never land after the next on
        std::sort (out.begin(), out.end(), [] (auto& a, auto& b) { return a.beat < b.beat; });
        for (size_t i = 0; i < out.size(); ++i)
            for (size_t j = i + 1; j < out.size(); ++j)
                if (out[j].note == out[i].note) { if (out[i].beat + out[i].len > out[j].beat) out[i].len = juce::jmax (0.01, out[j].beat - out[i].beat - 0.005); break; }
        for (auto& n : out)
        {
            const double on = std::round (n.beat * tpq), off = juce::jmax (on + 1.0, std::round ((n.beat + n.len) * tpq));
            seq.addEvent (juce::MidiMessage::noteOn (1, n.note, n.vel), on);
            seq.addEvent (juce::MidiMessage::noteOff (1, n.note), off);
        }
        seq.updateMatchedPairs();
        juce::MidiMessageSequence tempo;
        tempo.addEvent (juce::MidiMessage::tempoMetaEvent ((int) (60000000.0 / juce::jlimit (30.0, 300.0, bpm))), 0);
        tempo.addEvent (juce::MidiMessage::timeSignatureMetaEvent (4, 4), 0);
        juce::MidiFile mf; mf.setTicksPerQuarterNote (tpq);
        mf.addTrack (tempo); mf.addTrack (seq);
        file.deleteFile();
        juce::FileOutputStream os (file);
        if (os.openedOk()) { mf.writeTo (os, 1); os.flush(); }
        return file;
    }
}
