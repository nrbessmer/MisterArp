// MAVideo: renders the Mister Arp demo offline with the real processor + editor.
// A chord progression is held while the timeline switches presets and edits steps,
// so the video is the arp itself; a quiet drum bed sits underneath.
// Usage: MAVideo bed.wav outDir [scale] -> outDir/f%05d.png + audio.wav + captions.txt
#include <juce_audio_utils/juce_audio_utils.h>
#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"
#include <cstdio>

struct Host : juce::AudioPlayHead
{
    double bpm = 128.0, ppq = 0.0;
    juce::Optional<PositionInfo> getPosition() const override
    { PositionInfo p; p.setBpm (bpm); p.setPpqPosition (ppq); p.setIsPlaying (true); return p; }
};

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    const double SR = 48000.0; const int FPS = 30, spf = 1600, B = 400;
    const juce::File bedFile (argv[1]), outDir (argv[2]);
    const float scale = argc > 3 ? (float) atof (argv[3]) : 1.0f;
    outDir.createDirectory();

    juce::AudioFormatManager fm; fm.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> rd (fm.createReaderFor (bedFile));
    juce::AudioBuffer<float> bed (2, (int) rd->lengthInSamples);
    rd->read (&bed, 0, (int) rd->lengthInSamples, 0, true, true);

    MisterArpProcessor proc;
    Host host; proc.setPlayHead (&host);
    proc.setPlayConfigDetails (0, 2, SR, B);
    proc.prepareToPlay (SR, B);
    std::unique_ptr<juce::AudioProcessorEditor> ed (proc.createEditor());
    auto* e = static_cast<MisterArpEditor*> (ed.get());
    auto preset = [&] (const char* name)
    {
        const auto& all = marp::presets();
        for (size_t i = 0; i < all.size(); ++i)
            if (juce::String (all[i].name) == name) { e->presetBar.box.setSelectedId ((int) i + 1, juce::sendNotificationSync); e->grid.repaint(); return; }
        printf ("missing preset %s\n", name);
    };

    // Am - F - C - G, one bar each, held as chords (the arp does the rest)
    const int chords[4][3] = { { 57, 60, 64 }, { 53, 57, 60 }, { 55, 60, 64 }, { 55, 59, 62 } };
    const double beatSec = 60.0 / 128.0, barSec = 4 * beatSec;

    enum Action { None, Ratchets, Sound };
    struct Seg { const char* preset; int bars; Action act; const char* cap; bool singleNote; };
    const Seg segs[] = {
        { "Trance Gate Arp",   4, None,     "Trance Gate Arp  -  hold a chord, Mister Arp plays it", false },
        { "Trance Gate Arp",   2, Ratchets, "Click RATCHET on a step: 2-4 fast repeats inside it", false },
        { "Psy Rolling Arp",   4, None,     "Psy Rolling Arp  -  accents, ratchets, F# Phrygian scale", false },
        { "Deep House Stab Arp", 4, None,   "Chord mode + swing  -  Deep House Stab Arp", false },
        { "Glass Cascade",     4, None,     "Glass Cascade  -  FM bells falling over 3 octaves", false },
        { "Melody Maker",      4, None,     "Melody Maker  -  one held note, per-step transposes make the tune", true },
        { "Synthwave Drive",   2, Sound,    "SOUND page: every synth control. DRAG MIDI drops the arp into your DAW", false } };
    const int nSeg = 7;
    int totalBars = 0; for (auto& s : segs) totalBars += s.bars;
    const double tEnd = totalBars * barSec + 1.5;
    const int nFrames = (int) (tEnd * FPS);

    struct Ev { double t; int note; bool on; };
    std::vector<Ev> evs;
    std::vector<double> segStart;
    { int bar = 0;
      for (auto& s : segs)
      {
          segStart.push_back (bar * barSec);
          for (int b = 0; b < s.bars; ++b, ++bar)
          {
              const double t0 = bar * barSec + 0.001, t1 = (bar + 1) * barSec - 0.012;
              if (s.singleNote) { evs.push_back ({ t0, 57, true }); evs.push_back ({ t1, 57, false }); }
              else for (int n : chords[b % 4]) { evs.push_back ({ t0, n, true }); evs.push_back ({ t1, n, false }); }
          }
      }
      segStart.push_back (bar * barSec); }
    std::stable_sort (evs.begin(), evs.end(), [] (auto& a, auto& b) { return a.t < b.t || (a.t == b.t && ! a.on && b.on); });

    juce::AudioBuffer<float> out (2, nFrames * spf); out.clear();
    juce::AudioBuffer<float> blk (2, B);
    juce::String caps;
    int cur = -1; size_t ei = 0;
    for (int fr = 0; fr < nFrames; ++fr)
    {
        const double t = (double) fr / FPS;
        int seg = 0; while (seg < nSeg - 1 && t >= segStart[(size_t) seg + 1]) ++seg;
        if (seg != cur)
        {
            cur = seg; preset (segs[seg].preset);
            e->showPage (segs[seg].act == Sound ? 1 : 0);
            if (segs[seg].act == Ratchets)
                for (int i : { 3, 7, 11, 13, 14, 15 }) { auto s = proc.getStep (i); s.ratchet = i >= 13 ? 2 + (i - 13) : 2; proc.setStep (i, s); }
            caps << juce::String (segStart[(size_t) seg], 3) << "\t" << juce::String (juce::jmin (tEnd, segStart[(size_t) seg + 1] + (seg == nSeg - 1 ? 1.5 : 0.0)), 3)
                 << "\t" << segs[seg].cap << "\n";
        }
        for (int k = 0; k < spf; k += B)
        {
            const int s0 = fr * spf + k;
            juce::MidiBuffer mb;
            while (ei < evs.size() && (int) (evs[ei].t * SR) < s0 + B)
            {
                const auto& ev = evs[ei];
                mb.addEvent (ev.on ? juce::MidiMessage::noteOn (1, ev.note, 0.85f) : juce::MidiMessage::noteOff (1, ev.note), juce::jmax (0, (int) (ev.t * SR) - s0));
                ++ei;
            }
            blk.clear();
            proc.processBlock (blk, mb);
            host.ppq += B / SR * 128.0 / 60.0;
            for (int c = 0; c < 2; ++c)
            {
                out.addFrom (c, s0, blk, c, 0, B);
                if (s0 + B <= bed.getNumSamples()) out.addFrom (c, s0, bed, c, s0, B, 0.32f);
            }
        }
        e->timerCallback();
        auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, scale);
        juce::File f = outDir.getChildFile (juce::String::formatted ("f%05d.png", fr)); f.deleteFile();
        juce::FileOutputStream fos (f); juce::PNGImageFormat().writeImageToStream (img, fos);
    }
    outDir.getChildFile ("captions.txt").replaceWithText (caps);
    juce::WavAudioFormat wav; auto wf = outDir.getChildFile ("audio.wav"); wf.deleteFile();
    std::unique_ptr<juce::AudioFormatWriter> w (wav.createWriterFor (new juce::FileOutputStream (wf), SR, 2, 24, {}, 0));
    out.applyGain (0.89f / juce::jmax (0.05f, out.getMagnitude (0, out.getNumSamples())));
    w->writeFromAudioSampleBuffer (out, 0, out.getNumSamples());
    printf ("frames %d, %.1f s\n", nFrames, tEnd);
    proc.setPlayHead (nullptr);
    return 0;
}
