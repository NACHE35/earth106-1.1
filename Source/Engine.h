#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <cmath>
#include <algorithm>

namespace E106
{
struct Snap
{
    int w[2] {}, oct[2] {}, semi[2] {};
    float det[2] {}, lvl[2] {}, pw[2] { .5f, .5f }, a[2] {}, dc[2] {}, su[2] {}, r[2] {};
    float sub = 0, noise = 0, drive = 0, fEnv = 0, lfoP = 0, lfoF = 0, fc = .5f, res = 0, port = 0;
    float fA = 0, fD = 0, fS = 0, fR = 0, lfoR = 4, master = .5f, dlT = .3f, dlF = .3f, dlM = 0;
    bool hp = false; int poly = 5, ch = 0;
};
struct Ctx { float sr, lfo, cut, res, pc, bend; };

struct Env
{
    enum { Idle, Att, Dec, Sus, Rel };
    int st = Idle; float v = 0, inc = 0, dcy = 0, sus = 1, rc = 0;
    void on (float a, float d, float s, float sr)
    { st = Att; inc = 1.f / (std::max (a, .001f) * sr); dcy = std::exp (-1.f / (std::max (d, .001f) / 3.f * sr)); sus = s; }
    void off (float r, float sr) { if (st != Idle) { st = Rel; rc = std::exp (-1.f / (std::max (r, .001f) / 3.f * sr)); } }
    float tick()
    {
        switch (st)
        {
            case Att: v += inc; if (v >= 1.f) { v = 1.f; st = Dec; } break;
            case Dec: v = sus + (v - sus) * dcy; if (std::abs (v - sus) < 1e-4f) { v = sus; st = Sus; } break;
            case Rel: v *= rc; if (v < 1e-4f) { v = 0; st = Idle; } break;
            default: break;
        }
        return v;
    }
    bool active() const { return st != Idle; }
};

// Oscillateur PolyBLEP : 0 = scie, 1 = pulse, 2 = triangle, 3 = sinus
struct Osc
{
    float ph = 0;
    static float blep (float t, float dt)
    {
        if (t < dt) { t /= dt; return t + t - t * t - 1.f; }
        if (t > 1.f - dt) { t = (t - 1.f) / dt; return t * t + t + t + 1.f; }
        return 0.f;
    }
    float tick (int wave, float dt, float pw)
    {
        float y;
        switch (wave)
        {
            case 0: y = 2.f * ph - 1.f; y -= blep (ph, dt); break;
            case 1: y = ph < pw ? 1.f : -1.f; y += blep (ph, dt); y -= blep (std::fmod (ph + 1.f - pw, 1.f), dt); y -= (2.f * pw - 1.f); break;
            case 2: y = 4.f * std::abs (ph - .5f) - 1.f; break;
            default: y = std::sin (juce::MathConstants<float>::twoPi * ph); break;
        }
        ph += dt; if (ph >= 1.f) ph -= 1.f;
        return y;
    }
};

// Filtre 12 dB non linéaire (TPT SVF + saturation dans la boucle de résonance),
// inspiré du caractère agressif du filtre Korg MS-20. LP ou HP.
struct Filt
{
    float s1 = 0, s2 = 0, g = .1f, k = 2.f;
    void set (float fc, float res, float sr) { g = std::tan (juce::MathConstants<float>::pi * fc / sr); k = 2.f - 1.98f * res; }
    float run (float x, bool hp, float drive)
    {
        x = std::tanh (x * drive);
        const float hpv = (x - (k + g) * std::tanh (s1) - s2) / (1.f + k * g + g * g);
        const float bp = g * hpv + s1, lp = g * bp + s2;
        s1 = juce::jlimit (-4.f, 4.f, g * hpv + bp);
        s2 = juce::jlimit (-4.f, 4.f, g * bp + lp);
        return std::tanh ((hp ? hpv : lp) * .8f);
    }
};

struct Voice
{
    bool on = false, released = false; int note = 0, cc = 0; uint32_t age = 0;
    float vel = 1, f = 440, tf = 440, amp = .2f;
    Osc o[2], sub; Env e[2], fe; Filt fl;

    void start (int n, float v, float from, float to, const Snap& s, float sr, uint32_t a)
    {
        if (! on) { o[0].ph = o[1].ph = sub.ph = 0; fl.s1 = fl.s2 = 0; }
        on = true; released = false; note = n; vel = v; f = from; tf = to; age = a; cc = 0; amp = .2f * (.5f + .5f * v);
        for (int k = 0; k < 2; ++k) e[k].on (s.a[k], s.dc[k], s.su[k], sr);
        fe.on (s.fA, s.fD, s.fS, sr);
    }
    void stop (const Snap& s, float sr) { released = true; e[0].off (s.r[0], sr); e[1].off (s.r[1], sr); fe.off (s.fR, sr); }
    void kill (float sr) { released = true; e[0].off (.012f, sr); e[1].off (.012f, sr); fe.off (.012f, sr); }

    float render (const Snap& s, const Ctx& c, juce::Random& rnd)
    {
        f += (tf - f) * c.pc;
        float sum = 0;
        for (int k = 0; k < 2; ++k)
        {
            const float ev = e[k].tick();
            const float cents = s.oct[k] * 1200.f + s.semi[k] * 100.f + s.det[k] + c.lfo * s.lfoP * 100.f + c.bend;
            const float dt = std::min (.45f, f * std::exp2 (cents / 1200.f) / c.sr);
            float y = o[k].tick (s.w[k], dt, s.pw[k]) * s.lvl[k];
            if (k == 0 && s.sub > 0.f) y += s.sub * .6f * sub.tick (1, std::min (.45f, f * .5f * std::exp2 (c.bend / 1200.f) / c.sr), .5f);
            if (k == 1 && s.noise > 0.f) y += s.noise * .5f * (rnd.nextFloat() * 2.f - 1.f);
            sum += y * ev;
        }
        const float fenv = fe.tick();
        if ((cc++ & 15) == 0)
        {
            const float cents = (s.hp ? -1.f : 1.f) * s.fEnv * 4800.f * fenv + c.lfo * s.lfoF * 2400.f;
            fl.set (std::clamp (c.cut * std::exp2 (cents / 1200.f), 20.f, .45f * c.sr), c.res, c.sr);
        }
        const float out = fl.run (sum, s.hp, 1.f + s.drive * 5.f) * .7f * amp;
        if (released && ! e[0].active() && ! e[1].active()) on = false;
        return out;
    }
};
} // namespace E106
