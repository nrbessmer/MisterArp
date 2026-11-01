/*
  ==============================================================================
    Mister Arp - ArpPresets.h
    (c)(p) 2026 Tully EDM Vibe

    Sound + arp settings + a 16-step pattern ("steps=", see setPatternFromCode):
      x on   X accent   o soft   . rest   _ tie   rN ratchet   pNN probability   +N/-N transpose
    Hold a chord to hear them.
  ==============================================================================
*/
#pragma once
#include "SynthParams.h"

namespace marp
{
    inline const std::vector<tsyn::Preset>& presets()
    {
        static const std::vector<tsyn::Preset> p = {
        // ------------------------------------------------------------------ TRANCE
        { "Trance Gate Arp", "Trance", "Rolling 16th supersaw pluck, accents on the beat. Hold a minor chord.",
          "osc1_wave=SuperSaw osc1_detune=0.4 cutoff=900 reso=0.2 flt_env=0.55 flt_d=0.2 flt_s=0 amp_d=0.3 amp_s=0 amp_r=0.25 delay=0.3 delay_time=1/8D delay_fb=0.4 reverb=0.3 width=1.3 "
          "arp_mode=Up arp_rate=1/16 arp_oct=2 arp_gate=0.55 steps=X,x,o,x,X,x,o,x,X,x,o,x,X,x,o,x volume=-2.7" },
        { "Uplifting Climb", "Trance", "Up/Down over 3 octaves with a dotted delay: the breakdown classic.",
          "osc1_wave=SuperSaw osc1_detune=0.45 osc2_wave=Saw osc2_oct=1 osc2_level=0.3 cutoff=1500 flt_env=0.45 flt_d=0.25 flt_s=0.1 amp_d=0.35 amp_s=0 amp_r=0.3 delay=0.35 delay_time=1/8D delay_fb=0.45 reverb=0.45 reverb_size=0.85 "
          "arp_mode=Up/Down arp_rate=1/16 arp_oct=3 arp_gate=0.6 steps=X,x,x,x,X,x,x,x,X,x,x,x,X,x,x,x volume=-4.0" },
        { "Offbeat Trance", "Trance", "Rests on the beat, notes on the offbeats.",
          "osc1_wave=Saw osc1_unison=3 osc1_detune=0.25 cutoff=1100 reso=0.3 flt_env=0.5 flt_d=0.15 flt_s=0 amp_d=0.25 amp_s=0 delay=0.25 delay_time=1/8D reverb=0.3 "
          "arp_mode=Up arp_rate=1/16 arp_oct=2 arp_gate=0.5 steps=.,x,X,x,.,x,X,x,.,x,X,x,.,x,X,x volume=-0.2" },
        { "Vocal Trance Arp", "Trance", "Soft FM bell arp, wide and airy.",
          "osc1_wave=FM osc1_shape=0.3 fm_ratio=4:1 osc2_wave=Sine osc2_level=0.5 cutoff=9000 flt_env=0 amp_d=0.6 amp_s=0 amp_r=0.5 chorus=0.3 delay=0.3 delay_time=1/8D reverb=0.45 "
          "arp_mode=Up arp_rate=1/8 arp_oct=2 arp_gate=0.8 steps=X,x,x,x,X,x,x,x,X,x,x,x,X,x,x,x volume=-3.9" },
        { "Ratchet Riser", "Trance", "Ratchets on the last steps for a build-up feel.",
          "osc1_wave=SuperSaw osc1_detune=0.4 cutoff=1200 flt_env=0.5 flt_d=0.18 flt_s=0 amp_d=0.25 amp_s=0 reverb=0.3 delay=0.2 "
          "arp_mode=Up arp_rate=1/16 arp_oct=2 arp_gate=0.5 steps=X,x,x,x,X,x,x,x,X,x,x,x,Xr2,xr2,xr3,xr4 volume=-1.5" },
        { "Classic 90s Arp", "Trance", "Square-saw pluck, Down/Up, the old-school way.",
          "osc1_wave=Saw osc2_wave=Square osc2_oct=-1 osc2_level=0.4 cutoff=1400 reso=0.35 flt_env=0.4 flt_d=0.2 flt_s=0 amp_d=0.25 amp_s=0 delay=0.3 delay_time=1/8D "
          "arp_mode=Down/Up arp_rate=1/16 arp_oct=2 arp_gate=0.5 volume=3.0" },
        { "Euphoric 8ths", "Trance", "Slower 8th-note arp with a long tail.",
          "osc1_wave=SuperSaw osc1_detune=0.5 cutoff=2500 flt_env=0.3 flt_d=0.4 flt_s=0.2 amp_d=0.6 amp_s=0.1 amp_r=0.6 chorus=0.3 reverb=0.5 reverb_size=0.9 delay=0.25 "
          "arp_mode=Up/Down arp_rate=1/8 arp_oct=2 arp_gate=0.8 steps=X,x,x,x,x,x,x,x,X,x,x,x,x,x,x,x volume=-4.1" },
        { "Pinky Lead Arp", "Trance", "Pinky mode: every note alternates with the top note.",
          "osc1_wave=Saw osc1_unison=3 osc1_detune=0.3 cutoff=1800 flt_env=0.4 flt_d=0.2 flt_s=0.1 amp_d=0.3 amp_s=0.1 delay=0.3 delay_time=1/8D reverb=0.3 "
          "arp_mode=Pinky arp_rate=1/16 arp_oct=1 arp_gate=0.55 volume=-2.3" },

        // ------------------------------------------------------------------ PSY
        { "Psy Rolling Arp", "Psy", "Fast resonant 16ths with accent and ratchet for psytrance.",
          "voice_mode=Mono osc1_wave=Saw cutoff=800 reso=0.6 flt_env=0.6 flt_d=0.12 flt_s=0 amp_d=0.15 amp_s=0.2 drive_type=Hard drive=0.25 delay=0.25 delay_time=1/16D delay_fb=0.4 "
          "arp_mode=Up arp_rate=1/16 arp_oct=2 arp_gate=0.45 arp_scale=Phrygian arp_key=F# steps=X,x,o,x,X,x,o,xr2,X,x,o,x,X,x+12,o,x volume=3.0" },
        { "Goa Spiral", "Psy", "Converge mode squelch with a 16th delay.",
          "voice_mode=Mono osc1_wave=Saw osc2_wave=Pulse osc2_shape=0.3 osc2_level=0.5 cutoff=1200 reso=0.55 flt_env=0.5 flt_d=0.15 flt_s=0 lfo_rate=0.4 lfo_shapemod=0.4 delay=0.3 delay_time=1/16D "
          "arp_mode=Converge arp_rate=1/16 arp_oct=2 arp_gate=0.5 arp_scale=Harmonic_Minor volume=1.0" },
        { "Full-On Twister", "Psy", "FM zaps jumping octaves with probability.",
          "osc1_wave=FM osc1_shape=0.6 fm_ratio=3:1 cutoff=700 reso=0.5 flt_env=0.75 flt_d=0.1 flt_s=0 amp_d=0.12 amp_s=0 delay=0.3 delay_time=1/16D "
          "arp_mode=Random arp_rate=1/16 arp_oct=3 arp_gate=0.4 steps=X,x,x,xp60,X,x,xp60,x,X,x,x,xp60,X,xr2,x,xp60 volume=6.0" },
        { "Dark Forest", "Psy", "Low, folded and random in Phrygian.",
          "voice_mode=Mono osc1_wave=Fold osc1_shape=0.6 cutoff=1400 reso=0.55 flt_env=0.4 flt_d=0.12 amp_d=0.15 amp_s=0.2 drive_type=Hard drive=0.3 lfo_shape=S&H lfo_sync=1/16 lfo_shapemod=0.4 "
          "arp_mode=Random arp_rate=1/16 arp_oct=1 arp_gate=0.4 arp_scale=Phrygian steps=X,x,.,x,X,.,x,x,X,x,.,x,X,xr3,.,x volume=0.4" },
        { "Acid Arp", "Psy", "303-style arp with slides (gate over 100%) and accents.",
          "voice_mode=Legato glide=0.05 osc1_wave=Saw cutoff=420 reso=0.8 flt_env=0.7 flt_d=0.18 flt_s=0 drive_type=Soft drive=0.4 delay=0.15 delay_time=1/16D "
          "arp_mode=As_Played arp_rate=1/16 arp_oct=2 arp_gate=1.1 steps=X,o,x,o,X,o,x_,_,X,o,x,o,X+12,o,x,o volume=-0.1" },
        { "Hypno Triplets", "Psy", "16th triplets with a tempo-synced filter wobble.",
          "voice_mode=Mono osc1_wave=Square cutoff=900 reso=0.5 flt_env=0.4 flt_d=0.12 lfo_shape=Triangle lfo_sync=1/2 lfo_cutoff=0.3 amp_d=0.2 amp_s=0.3 delay=0.2 delay_time=1/8D "
          "arp_mode=Up/Down arp_rate=1/16T arp_oct=2 arp_gate=0.5 arp_steps=12 volume=-0.7" },
        { "Zen Psy Pluck", "Psy", "Softer psy pluck in minor pentatonic.",
          "osc1_wave=FM osc1_shape=0.3 fm_ratio=2:1 cutoff=2000 flt_env=0.4 flt_d=0.15 flt_s=0 amp_d=0.2 amp_s=0 delay=0.35 delay_time=1/8D delay_fb=0.45 reverb=0.3 "
          "arp_mode=Diverge arp_rate=1/16 arp_oct=2 arp_gate=0.5 arp_scale=Minor_Pentatonic volume=-3.0" },
        { "Thumb Bounce", "Psy", "Thumb mode: the bottom note bounces under the others.",
          "voice_mode=Mono osc1_wave=Saw cutoff=1000 reso=0.45 flt_env=0.5 flt_d=0.12 flt_s=0 amp_d=0.15 amp_s=0.1 drive_type=Soft drive=0.3 delay=0.2 delay_time=1/16D "
          "arp_mode=Thumb arp_rate=1/16 arp_oct=2 arp_gate=0.45 volume=4.5" },

        // ------------------------------------------------------------------ HOUSE & TECHNO
        { "Deep House Stab Arp", "House & Techno", "Swung chord stabs, deep and round.",
          "osc1_wave=Saw osc2_wave=Square osc2_oct=-1 osc2_level=0.4 cutoff=500 reso=0.35 flt_env=0.5 flt_d=0.25 flt_s=0 amp_d=0.35 amp_s=0 chorus=0.3 reverb=0.3 "
          "arp_mode=Chord arp_rate=1/16 arp_gate=0.5 arp_swing=0.35 steps=.,.,X,.,.,x,.,.,.,.,X,.,.,x,.,x volume=-2.2" },
        { "Melodic Techno", "House & Techno", "Dark saw arp, long delay, Afterlife style.",
          "osc1_wave=Saw osc1_unison=3 osc1_detune=0.2 cutoff=600 reso=0.4 flt_env=0.45 flt_d=0.3 flt_s=0 amp_d=0.4 amp_s=0 delay=0.4 delay_time=1/8D delay_fb=0.55 reverb=0.4 reverb_size=0.85 "
          "arp_mode=Up arp_rate=1/16 arp_oct=2 arp_gate=0.55 arp_scale=Minor steps=X,x,x,x,X,x,x,.,X,x,x,x,X,x,.,x volume=-2.9" },
        { "Garage Organ", "House & Techno", "Swung organ-ish square arp.",
          "osc1_wave=Square osc2_wave=Sine osc2_oct=1 osc2_level=0.5 cutoff=3000 flt_env=0.2 amp_d=0.25 amp_s=0.3 amp_r=0.1 reverb=0.2 "
          "arp_mode=As_Played arp_rate=1/16 arp_oct=1 arp_gate=0.45 arp_swing=0.5 volume=-5.8" },
        { "Minimal Blip", "House & Techno", "Sparse sine blips with probability: never repeats the same way.",
          "osc1_wave=Sine cutoff=8000 flt_env=0 amp_d=0.12 amp_s=0 amp_r=0.08 delay=0.35 delay_time=1/8D delay_fb=0.5 reverb=0.25 "
          "arp_mode=Random arp_rate=1/16 arp_oct=2 arp_gate=0.3 steps=X,.,xp50,.,xp70,.,xp40,x,X,.,xp50,.,xp70,xp30,.,x volume=2.9" },
        { "Tech House Pluck", "House & Techno", "Short plucky square with swing.",
          "osc1_wave=Square cutoff=700 reso=0.3 flt_env=0.55 flt_d=0.12 flt_s=0 amp_d=0.18 amp_s=0 delay=0.2 delay_time=1/8D "
          "arp_mode=Up arp_rate=1/16 arp_oct=1 arp_gate=0.4 arp_swing=0.3 steps=X,.,x,x,.,x,X,.,x,.,x,x,.,x,X,. volume=3.2" },
        { "Peak Time Arp", "House & Techno", "Driven saw arp for big-room techno.",
          "osc1_wave=Saw osc1_unison=3 osc1_detune=0.25 cutoff=900 reso=0.45 flt_env=0.5 flt_d=0.15 flt_s=0 amp_d=0.2 amp_s=0 drive_type=Hard drive=0.3 reverb=0.3 delay=0.2 delay_time=1/16D "
          "arp_mode=Up/Down arp_rate=1/16 arp_oct=2 arp_gate=0.45 volume=-3.8" },
        { "Afro Marimba", "House & Techno", "Marimba-like arp with a swung 12/8 groove.",
          "osc1_wave=Sine osc2_wave=Triangle osc2_oct=1 osc2_level=0.4 cutoff=6000 flt_env=0 amp_d=0.3 amp_s=0 amp_r=0.25 reverb=0.3 "
          "arp_mode=As_Played arp_rate=1/8T arp_oct=2 arp_gate=0.6 arp_steps=12 steps=X,x,o,X,x,o,X,o,x,X,o,x volume=-3.0" },
        { "Rave Stab Chords", "House & Techno", "90s rave stabs in chord mode, octave hopping.",
          "osc1_wave=Saw osc1_unison=3 osc1_detune=0.3 osc2_wave=Square osc2_level=0.5 cutoff=2500 flt_env=0.3 flt_d=0.2 amp_d=0.3 amp_s=0 reverb=0.3 "
          "arp_mode=Chord arp_rate=1/8 arp_oct=2 arp_gate=0.5 steps=X,.,x,.,X,x,.,x,X,.,x,.,X,.,x,x volume=-4.8" },

        // ------------------------------------------------------------------ MELODIC & AMBIENT
        { "Glass Cascade", "Melodic & Ambient", "Bell arp falling down three octaves into reverb.",
          "osc1_wave=FM osc1_shape=0.3 fm_ratio=7:1 cutoff=12000 flt_env=0 amp_d=1.2 amp_s=0 amp_r=1 reverb=0.55 reverb_size=0.9 delay=0.3 delay_time=1/4D "
          "arp_mode=Down arp_rate=1/16 arp_oct=3 arp_gate=0.7 volume=-10.7" },
        { "Music Box", "Melodic & Ambient", "Tiny, sweet sine-and-bell arp.",
          "osc1_wave=Sine osc2_wave=FM osc2_shape=0.25 osc2_level=0.4 osc2_oct=1 fm_ratio=4:1 cutoff=9000 flt_env=0 amp_d=0.6 amp_s=0 amp_r=0.6 reverb=0.4 "
          "arp_mode=Up/Down arp_rate=1/8 arp_oct=2 arp_gate=0.7 arp_scale=Major_Pentatonic volume=-4.1" },
        { "Slow Bloom", "Melodic & Ambient", "Slow-attack 8th arp that melts into a pad.",
          "osc1_wave=SuperSaw osc1_detune=0.4 amp_a=0.08 amp_d=0.8 amp_s=0.5 amp_r=1.5 cutoff=2500 flt_env=0.15 chorus=0.4 reverb=0.6 reverb_size=0.95 "
          "arp_mode=As_Played arp_rate=1/8 arp_oct=2 arp_gate=1.2 volume=-8.7" },
        { "Cinematic Pulse", "Melodic & Ambient", "Steady 8th pulse with a slow filter LFO.",
          "osc1_wave=Saw osc2_wave=Saw osc2_oct=-1 osc2_level=0.5 cutoff=1200 reso=0.25 lfo_shape=Sine lfo_rate=0.1 lfo_cutoff=0.4 amp_d=0.3 amp_s=0.2 reverb=0.45 "
          "arp_mode=Up arp_rate=1/8 arp_oct=1 arp_gate=0.6 volume=-0.9" },
        { "Harp Arp", "Melodic & Ambient", "Triangle harp, three octaves up and down.",
          "osc1_wave=Triangle osc2_wave=Saw osc2_oct=-1 osc2_level=0.15 cutoff=3500 flt_env=0.3 flt_d=0.4 flt_s=0 amp_d=0.9 amp_s=0 amp_r=0.7 reverb=0.45 "
          "arp_mode=Up/Down arp_rate=1/16 arp_oct=3 arp_gate=0.9 volume=-6.6" },
        { "Ambient Drift", "Melodic & Ambient", "Random notes with probability and a huge reverb.",
          "osc1_wave=Sine osc2_wave=Triangle osc2_oct=1 osc2_level=0.3 cutoff=5000 flt_env=0 amp_a=0.02 amp_d=1.5 amp_s=0 amp_r=1.5 reverb=0.7 reverb_size=1 delay=0.35 delay_time=1/4D delay_fb=0.55 "
          "arp_mode=Random arp_rate=1/8 arp_oct=3 arp_gate=0.9 arp_scale=Major_Pentatonic steps=X,.,xp50,.,xp60,.,.,xp40,x,.,xp50,.,xp60,.,.,xp40 volume=-4.2" },
        { "Lo-Fi Keys Arp", "Melodic & Ambient", "Warm FM keys with tremolo and swing.",
          "osc1_wave=FM osc1_shape=0.25 fm_ratio=1:1 cutoff=4000 flt_env=0 amp_d=1 amp_s=0.2 amp_r=0.4 lfo_rate=4 lfo_amp=0.2 chorus=0.4 drive_type=Crush drive=0.1 reverb=0.3 "
          "arp_mode=As_Played arp_rate=1/8 arp_oct=1 arp_gate=0.8 arp_swing=0.4 volume=-10.5" },

        // ------------------------------------------------------------------ SYNTHWAVE & RETRO
        { "Synthwave Drive", "Synthwave & Retro", "Chorused 16th saw arp, straight out of 1985.",
          "osc1_wave=Saw osc1_unison=3 osc1_detune=0.2 cutoff=1500 flt_env=0.4 flt_d=0.2 flt_s=0.1 amp_d=0.25 amp_s=0.2 chorus=0.5 delay=0.3 delay_time=1/8D reverb=0.4 "
          "arp_mode=Up arp_rate=1/16 arp_oct=2 arp_gate=0.6 volume=-2.1" },
        { "Berlin Sequence", "Synthwave & Retro", "Tangerine-style 16th sequence with filter movement.",
          "voice_mode=Mono osc1_wave=Saw osc2_wave=Pulse osc2_shape=0.3 osc2_level=0.5 cutoff=800 reso=0.45 flt_env=0.5 flt_d=0.15 flt_s=0 lfo_rate=0.15 lfo_cutoff=0.35 delay=0.3 delay_time=1/8D reverb=0.35 "
          "arp_mode=As_Played arp_rate=1/16 arp_oct=2 arp_gate=0.5 steps=X,x,x,X,x,x,X,x,X,x,x,X,x,x,X,x volume=-4.9" },
        { "Chiptune Arp", "Synthwave & Retro", "Fast 32nd chord arp like an old game console.",
          "osc1_wave=Pulse osc1_shape=0.25 flt_type=LP_12 cutoff=20000 flt_env=0 amp_d=0.1 amp_s=0.6 amp_r=0.02 drive_type=Crush drive=0.3 "
          "arp_mode=Up arp_rate=1/32 arp_oct=1 arp_gate=0.9 volume=0.1" },
        { "Retro Bass Arp", "Synthwave & Retro", "Octave-jumping bass arp for driving tracks.",
          "voice_mode=Mono osc1_wave=Saw osc2_wave=Square osc2_oct=-1 osc2_level=0.6 cutoff=600 reso=0.3 flt_env=0.45 flt_d=0.15 flt_s=0.1 amp_d=0.2 amp_s=0.4 "
          "arp_mode=Up arp_rate=1/16 arp_oct=2 arp_gate=0.6 steps=X,x+12,x,x+12,X,x+12,x,x+12,X,x+12,x,x+12,X,x+12,x,x+12 volume=0.7" },
        { "80s Bell Arp", "Synthwave & Retro", "DX-style bell arp with chorus.",
          "osc1_wave=FM osc1_shape=0.4 fm_ratio=3:1 cutoff=8000 flt_env=0 amp_d=0.5 amp_s=0 amp_r=0.4 chorus=0.5 reverb=0.4 delay=0.25 "
          "arp_mode=Up/Down arp_rate=1/16 arp_oct=2 arp_gate=0.6 volume=-5.3" },
        { "Outrun Gallop", "Synthwave & Retro", "Galloping 8th + two 16ths rhythm.",
          "osc1_wave=Saw osc1_unison=3 osc1_detune=0.25 cutoff=1800 flt_env=0.35 flt_d=0.15 amp_d=0.2 amp_s=0.2 chorus=0.4 reverb=0.35 "
          "arp_mode=Up arp_rate=1/16 arp_oct=2 arp_gate=0.5 steps=X,.,x,x,X,.,x,x,X,.,x,x,X,.,x,x volume=-0.0" },

        // ------------------------------------------------------------------ CHORDS & STABS
        { "Supersaw Chords", "Chords & Stabs", "Chord mode: the whole chord on every step.",
          "osc1_wave=SuperSaw osc1_detune=0.45 cutoff=4000 flt_env=0.2 amp_d=0.3 amp_s=0.2 reverb=0.35 width=1.3 "
          "arp_mode=Chord arp_rate=1/8 arp_gate=0.5 steps=X,.,x,.,X,x,.,x,X,.,x,.,X,.,x,. volume=-4.0" },
        { "Pumping Chords", "Chords & Stabs", "Sidechain-style chord pumps on every 8th.",
          "osc1_wave=SuperSaw osc1_detune=0.4 cutoff=6000 amp_a=0.03 amp_d=0.25 amp_s=0.3 reverb=0.3 "
          "arp_mode=Chord arp_rate=1/8 arp_gate=0.85 volume=-6.4" },
        { "Reggaeton Stab", "Chords & Stabs", "Dembow-ish chord rhythm.",
          "osc1_wave=Saw osc2_wave=Square osc2_level=0.5 cutoff=2500 flt_env=0.3 flt_d=0.15 amp_d=0.2 amp_s=0 reverb=0.2 "
          "arp_mode=Chord arp_rate=1/16 arp_gate=0.5 steps=X,.,.,x,.,.,X,.,X,.,.,x,.,.,X,. volume=-3.9" },
        { "House Piano Stabs", "Chords & Stabs", "FM piano chords, offbeat house rhythm.",
          "osc1_wave=FM osc1_shape=0.3 fm_ratio=1:1 osc2_wave=Triangle osc2_level=0.4 cutoff=6000 flt_env=0 amp_d=0.5 amp_s=0 amp_r=0.3 reverb=0.25 "
          "arp_mode=Chord arp_rate=1/8 arp_gate=0.5 steps=.,X,.,x,.,X,.,x,.,X,.,x,.,X,x,x volume=-4.5" },
        { "Trance Gate Chords", "Chords & Stabs", "16th gated chords with accents.",
          "osc1_wave=SuperSaw osc1_detune=0.4 cutoff=7000 amp_d=0.1 amp_s=0.8 amp_r=0.05 reverb=0.3 "
          "arp_mode=Chord arp_rate=1/16 arp_gate=0.6 steps=X,x,.,x,X,.,x,x,X,x,.,x,X,.,x,x volume=-7.6" },

        // ------------------------------------------------------------------ EXPERIMENTAL
        { "Probability Rain", "Experimental", "Every step has a chance to play: always different.",
          "osc1_wave=FM osc1_shape=0.35 fm_ratio=5:1 cutoff=9000 flt_env=0 amp_d=0.4 amp_s=0 amp_r=0.4 reverb=0.5 delay=0.35 delay_time=1/8D delay_fb=0.5 "
          "arp_mode=Random arp_rate=1/16 arp_oct=3 arp_gate=0.5 arp_scale=Dorian steps=xp70,xp40,xp60,xp30,xp70,xp40,xp60,xp30,xp70,xp40,xp60,xp30,xp70,xp40,xp60,xp30 volume=-2.9" },
        { "Glitch Ratchets", "Experimental", "Ratchets and rests for stuttering IDM lines.",
          "osc1_wave=Fold osc1_shape=0.5 cutoff=3000 reso=0.3 flt_env=0.3 amp_d=0.12 amp_s=0 drive_type=Crush drive=0.25 delay=0.2 delay_time=1/16D "
          "arp_mode=Random arp_rate=1/16 arp_oct=2 arp_gate=0.4 steps=Xr3,.,x,xr2,.,Xr4,x,.,xr2,X,.,xr3,x,.,Xr2,x volume=6.0" },
        { "Polymeter 5", "Experimental", "5-step pattern against the 4/4 bar.",
          "osc1_wave=Square cutoff=1500 reso=0.35 flt_env=0.4 flt_d=0.12 amp_d=0.15 amp_s=0 delay=0.3 delay_time=1/8D reverb=0.3 "
          "arp_mode=Up arp_rate=1/16 arp_oct=2 arp_gate=0.5 arp_steps=5 steps=X,x,o,x+7,o volume=-3.1" },
        { "Polymeter 7", "Experimental", "7-step pattern drifting over the bar.",
          "osc1_wave=FM osc1_shape=0.4 fm_ratio=2:1 cutoff=3000 flt_env=0.3 flt_d=0.15 amp_d=0.2 amp_s=0 delay=0.3 reverb=0.3 "
          "arp_mode=Up/Down arp_rate=1/16 arp_oct=2 arp_gate=0.5 arp_steps=7 steps=X,x,o,x,X,o,x-5 volume=-2.5" },
        { "Melody Maker", "Experimental", "Transposes per step: one held note becomes a tune.",
          "osc1_wave=Saw osc1_unison=3 osc1_detune=0.2 cutoff=1800 flt_env=0.4 flt_d=0.2 amp_d=0.3 amp_s=0.1 delay=0.3 delay_time=1/8D reverb=0.35 "
          "arp_mode=Up arp_rate=1/16 arp_oct=1 arp_gate=0.6 arp_scale=Minor steps=X,x+3,x+7,x+10,X+12,x+10,x+7,x+3,X+5,x+8,x+12,x+8,X+3,x+7,x+5,x+2 volume=-3.1" },
        { "Tied Legato Line", "Experimental", "Ties make long notes inside a fast pattern.",
          "voice_mode=Legato glide=0.06 osc1_wave=Saw osc2_wave=Saw osc2_fine=8 osc2_level=0.6 cutoff=1600 reso=0.35 flt_env=0.35 lfo_rate=5.5 lfo_pitch=0.15 lfo_delay=0.3 reverb=0.3 "
          "arp_mode=As_Played arp_rate=1/16 arp_oct=2 arp_gate=0.9 steps=X,_,_,x,X,_,x,x,X,_,_,_,x,x,X,_ volume=-6.4" },
        };
        return p;
    }
}
