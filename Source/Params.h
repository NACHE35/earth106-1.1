#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <map>
#include <vector>

namespace E106
{
struct FDef { const char* id; const char* name; float mn, mx, def; char kind; bool ex; };
struct IDef { const char* id; const char* name; int mn, mx, def; };
struct CDef { const char* id; const char* name; std::vector<juce::String> items; int def; };

inline juce::String fmtV (char k, float v)
{
    switch (k)
    {
        case 'p': return juce::String (juce::roundToInt (v * 100.f)) + " %";
        case 's': return v < 1.f ? juce::String (juce::roundToInt (v * 1000.f)) + " ms" : juce::String (v, 2) + " s";
        case 'h': return juce::String (v, 1) + " Hz";
        default:  return (v > 0.f ? "+" : "") + juce::String (juce::roundToInt (v)) + " ct";
    }
}

inline const std::vector<FDef>& floatDefs()
{
    static const std::vector<FDef> d = {
        { "fc", "Cutoff", 0, 1, .62f, 'p', false }, { "res", "Resonance", 0, 1, .25f, 'p', false },
        { "port", "Portamento", 0, 1.5f, 0, 's', false }, { "master", "Master", 0, 1, .7f, 'p', false },
        { "sub", "Sub Osc", 0, 1, .3f, 'p', false }, { "noise", "Noise", 0, 1, 0, 'p', false },
        { "lfoR", "LFO Rate", .1f, 20, 4, 'h', true }, { "lfoP", "LFO Pitch", 0, 1, 0, 'p', false },
        { "lfoF", "LFO Filter", 0, 1, 0, 'p', false }, { "drive", "Drive", 0, 1, .25f, 'p', false },
        { "fEnv", "Filter Env Amt", -1, 1, .4f, 'p', false },
        { "fA", "F Attack", .003f, 4, .01f, 's', true }, { "fD", "F Decay", .003f, 4, .4f, 's', true },
        { "fS", "F Sustain", 0, 1, .3f, 'p', false }, { "fR", "F Release", .003f, 4, .3f, 's', true },
        { "d1", "Osc1 Detune", -50, 50, 0, 'c', false }, { "l1", "Osc1 Level", 0, 1, .7f, 'p', false },
        { "p1", "Osc1 Pulse W", .05f, .5f, .5f, 'p', false },
        { "a1", "Osc1 Attack", .003f, 4, .01f, 's', true }, { "dc1", "Osc1 Decay", .003f, 4, .4f, 's', true },
        { "su1", "Osc1 Sustain", 0, 1, .7f, 'p', false }, { "r1", "Osc1 Release", .003f, 4, .3f, 's', true },
        { "d2", "Osc2 Detune", -50, 50, 6, 'c', false }, { "l2", "Osc2 Level", 0, 1, .5f, 'p', false },
        { "p2", "Osc2 Pulse W", .05f, .5f, .5f, 'p', false },
        { "a2", "Osc2 Attack", .003f, 4, .01f, 's', true }, { "dc2", "Osc2 Decay", .003f, 4, .4f, 's', true },
        { "su2", "Osc2 Sustain", 0, 1, .7f, 'p', false }, { "r2", "Osc2 Release", .003f, 4, .3f, 's', true },
        { "dlT", "Delay Time", .02f, 1, .35f, 's', true }, { "dlF", "Delay Feedback", 0, .9f, .35f, 'p', false },
        { "dlM", "Delay Mix", 0, .6f, 0, 'p', false } };
    return d;
}
inline const std::vector<IDef>& intDefs()
{
    static const std::vector<IDef> d = { { "o1", "Osc1 Octave", -2, 2, 0 }, { "s1", "Osc1 Semitone", -12, 12, 0 },
                                         { "o2", "Osc2 Octave", -2, 2, 0 }, { "s2", "Osc2 Semitone", -12, 12, 0 } };
    return d;
}
inline const std::vector<CDef>& choiceDefs()
{
    static const std::vector<CDef> d = {
        { "w1", "Osc1 Wave", { "SAW", "PULSE", "TRI", "SINE" }, 0 }, { "w2", "Osc2 Wave", { "SAW", "PULSE", "TRI", "SINE" }, 1 },
        { "poly", "Polyphony", { "MONO", "2", "4", "6", "8", "16" }, 5 }, { "ch", "Chorus", { "OFF", "I", "II", "I+II" }, 3 } };
    return d;
}

inline juce::AudioProcessorValueTreeState::ParameterLayout makeLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout L;
    for (auto& d : floatDefs())
    {
        NormalisableRange<float> r (d.mn, d.mx);
        if (d.ex) r.setSkewForCentre (std::sqrt (d.mn * d.mx));
        const char k = d.kind;
        L.add (std::make_unique<AudioParameterFloat> (ParameterID { d.id, 1 }, d.name, r, d.def,
               AudioParameterFloatAttributes().withStringFromValueFunction ([k] (float v, int) { return fmtV (k, v); })));
    }
    for (auto& d : intDefs())
        L.add (std::make_unique<AudioParameterInt> (ParameterID { d.id, 1 }, d.name, d.mn, d.mx, d.def,
               AudioParameterIntAttributes().withStringFromValueFunction ([] (int v, int) { return (v > 0 ? "+" : "") + String (v); })));
    for (auto& d : choiceDefs())
        L.add (std::make_unique<AudioParameterChoice> (ParameterID { d.id, 1 }, d.name, StringArray (d.items.data(), (int) d.items.size()), d.def));
    L.add (std::make_unique<AudioParameterBool> (ParameterID { "hp", 1 }, "Low Cut (HP)", false));
    return L;
}
} // namespace E106
