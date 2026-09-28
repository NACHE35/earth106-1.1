#pragma once
#include "Params.h"

namespace E106
{
struct Preset { juce::String name; std::map<juce::String, float> v; };

inline std::map<juce::String, float> defaultsMap()
{
    std::map<juce::String, float> m;
    for (auto& d : floatDefs())  m[d.id] = d.def;
    for (auto& d : intDefs())    m[d.id] = (float) d.def;
    for (auto& d : choiceDefs()) m[d.id] = (float) d.def;
    m["hp"] = 0.f;
    return m;
}

struct Rng { uint32_t s; float operator()() { s += 0x6D2B79F5u; uint32_t t = s; t = (t ^ (t >> 15)) * (t | 1u); t ^= t + (t ^ (t >> 7)) * (t | 61u); return (float) ((t ^ (t >> 14)) / 4294967296.0); } };

inline std::vector<Preset> makeFactoryPresets()
{
    struct T { float fc = .5f, res = .2f; int w1 = 0, w2 = 1, o1 = 0, o2 = 0, s2 = 0; float a = .01f, dc = .4f, su = .7f, r = .3f, fEnv = .3f; int ch = 3, poly = 5; float sub = .2f, port = 0, lfoF = 0, lfoR = 4, noise = 0; };
    struct C { const char* n; T t; };
    const std::vector<C> cats = {
        { "Pad",     { .fc = .45f, .res = .2f,  .w1 = 0, .w2 = 1, .a = .8f,   .dc = 1,    .su = .8f, .r = 1.5f, .fEnv = .3f, .ch = 3, .poly = 5, .sub = .2f } },
        { "Brass",   { .fc = .35f, .res = .15f, .w1 = 0, .w2 = 0, .a = .08f,  .dc = .4f,  .su = .7f, .r = .2f,  .fEnv = .6f, .ch = 1, .poly = 3, .sub = .2f } },
        { "Bass",    { .fc = .3f,  .res = .45f, .w1 = 0, .w2 = 1, .o1 = -1, .o2 = -1, .a = .003f, .dc = .3f, .su = .4f, .r = .1f, .fEnv = .5f, .ch = 0, .poly = 0, .sub = .6f } },
        { "Lead",    { .fc = .6f,  .res = .3f,  .w1 = 0, .w2 = 0, .a = .01f,  .dc = .3f,  .su = .8f, .r = .2f,  .fEnv = .3f, .ch = 2, .poly = 0, .sub = 0, .port = .12f } },
        { "Strings", { .fc = .55f, .res = .1f,  .w1 = 0, .w2 = 0, .a = .5f,   .dc = 1,    .su = 1,   .r = .9f,  .fEnv = .1f, .ch = 3, .poly = 5, .sub = .1f } },
        { "Keys",    { .fc = .5f,  .res = .15f, .w1 = 2, .w2 = 1, .a = .005f, .dc = .8f,  .su = .3f, .r = .4f,  .fEnv = .3f, .ch = 1, .poly = 4, .sub = .1f } },
        { "Pluck",   { .fc = .45f, .res = .35f, .w1 = 1, .w2 = 0, .a = .003f, .dc = .25f, .su = 0,   .r = .2f,  .fEnv = .7f, .ch = 2, .poly = 4, .sub = .2f } },
        { "Sweep",   { .fc = .25f, .res = .6f,  .w1 = 0, .w2 = 0, .a = 1.2f,  .dc = 2,    .su = .7f, .r = 2,    .fEnv = .9f, .ch = 3, .poly = 5, .sub = .2f, .lfoF = .4f } },
        { "Bell",    { .fc = .7f,  .res = .1f,  .w1 = 3, .w2 = 3, .s2 = 12, .a = .003f, .dc = 2, .su = 0, .r = 2, .fEnv = .1f, .ch = 2, .poly = 5, .sub = 0 } },
        { "FX",      { .fc = .5f,  .res = .7f,  .w1 = 0, .w2 = 0, .a = .3f,   .dc = 1.5f, .su = .5f, .r = 1.5f, .fEnv = .8f, .ch = 3, .poly = 3, .sub = 0, .lfoF = .6f, .lfoR = 6, .noise = .5f } } };
    const char* adj[15] = { "Warm", "Dusty", "Neon", "Retro", "Miami", "Tokyo", "Cosmic", "Analog", "Glass", "Velvet", "Chrome", "Sunset", "Night", "Lunar", "Solar" };
    auto cl = [] (float x) { return juce::jlimit (0.f, 1.f, x); };

    std::vector<Preset> out;
    for (int ci = 0; ci < (int) cats.size(); ++ci)
        for (int i = 0; i < 15; ++i)
        {
            Rng r { (uint32_t) (ci * 97 + i * 13 + 5) };
            const T& b = cats[(size_t) ci].t;
            auto v = defaultsMap();
            v["fc"] = cl (b.fc + (r() - .5f) * .3f);   v["res"] = cl (b.res + (r() - .4f) * .3f);
            v["d2"] = std::round ((r() - .3f) * 24.f);
            v["ch"] = (b.ch && r() < .7f) ? (float) b.ch : (float) juce::jmin (3, (int) (r() * 4));
            v["w1"] = (float) b.w1;  v["w2"] = (r() < .35f) ? (float) juce::jmin (3, (int) (r() * 4)) : (float) b.w2;
            v["p1"] = .1f + r() * .4f; v["p2"] = .1f + r() * .4f;
            v["sub"] = cl (b.sub + (r() - .5f) * .3f);
            v["dlM"] = r() < .35f ? r() * .3f : 0.f;  v["dlT"] = .1f + r() * .5f;  v["dlF"] = .2f + r() * .4f;
            v["hp"] = (ci >= 7 && i % 7 == 6) ? 1.f : 0.f;
            v["a1"] = v["a2"] = b.a;  v["dc1"] = v["dc2"] = b.dc;  v["su1"] = v["su2"] = b.su;  v["r1"] = v["r2"] = b.r;
            v["fA"] = b.a * .6f + .003f;  v["fD"] = b.dc;  v["fS"] = b.su * .4f;  v["fR"] = b.r;  v["fEnv"] = b.fEnv;
            v["o1"] = (float) b.o1;  v["o2"] = (float) b.o2;  v["s2"] = (float) b.s2;  v["l2"] = .4f + r() * .4f;
            v["poly"] = (float) b.poly;  v["port"] = b.port;  v["lfoF"] = b.lfoF;  v["lfoR"] = b.lfoR;  v["noise"] = b.noise;
            Preset p; p.name = juce::String (out.size() + 1).paddedLeft ('0', 3) + " " + cats[(size_t) ci].n + " - " + adj[i]; p.v = v;
            out.push_back (p);
        }
    return out;
}
} // namespace E106
