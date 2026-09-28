#include "PluginProcessor.h"
#include "PluginEditor.h"
using namespace juce;

EarthProcessor::EarthProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", E106::makeLayout())
{
    presets = E106::makeFactoryPresets();
    nFactory = (int) presets.size();
    loadUser();
}

void EarthProcessor::prepareToPlay (double rate, int)
{
    sr = rate;
    fcS.reset (sr, .02); resS.reset (sr, .02); masterS.reset (sr, .02);
    auto s = readSnap();
    fcS.setCurrentAndTargetValue (s.fc); resS.setCurrentAndTargetValue (s.res); masterS.setCurrentAndTargetValue (s.master * s.master * .9f);
    cb.assign (4096, 0.f); dbL.assign ((size_t) (sr * 1.1) + 8, 0.f); dbR = dbL; cw = dw = 0; dsm = (float) (s.dlT * sr);
    for (auto& v : voices) v = E106::Voice();
}

E106::Snap EarthProcessor::readSnap()
{
    E106::Snap s;
    auto v = [this] (const String& id) { return apvts.getRawParameterValue (id)->load(); };
    for (int k = 0; k < 2; ++k)
    {
        const String x (k + 1);
        s.w[k] = (int) v ("w" + x); s.oct[k] = (int) v ("o" + x); s.semi[k] = (int) v ("s" + x);
        s.det[k] = v ("d" + x); s.lvl[k] = v ("l" + x); s.pw[k] = v ("p" + x);
        s.a[k] = v ("a" + x); s.dc[k] = v ("dc" + x); s.su[k] = v ("su" + x); s.r[k] = v ("r" + x);
    }
    s.sub = v ("sub"); s.noise = v ("noise"); s.drive = v ("drive"); s.fEnv = v ("fEnv"); s.lfoP = v ("lfoP"); s.lfoF = v ("lfoF");
    s.fc = v ("fc"); s.res = v ("res"); s.port = v ("port"); s.fA = v ("fA"); s.fD = v ("fD"); s.fS = v ("fS"); s.fR = v ("fR");
    s.lfoR = v ("lfoR"); s.master = v ("master"); s.dlT = v ("dlT"); s.dlF = v ("dlF"); s.dlM = v ("dlM");
    s.hp = v ("hp") > .5f; s.poly = (int) v ("poly"); s.ch = (int) v ("ch");
    return s;
}

void EarthProcessor::noteOn (int n, float vel, const E106::Snap& s)
{
    static const int mxT[] = { 1, 2, 4, 6, 8, 16 };
    const int mx = mxT[jlimit (0, 5, s.poly)];
    for (auto& v : voices) if (v.on && ! v.released && v.note == n) v.kill ((float) sr);
    int act = 0; for (auto& v : voices) if (v.on && ! v.released) ++act;
    while (act >= mx)
    {
        E106::Voice* o = nullptr;
        for (auto& v : voices) if (v.on && ! v.released && (o == nullptr || v.age < o->age)) o = &v;
        if (o == nullptr) break;
        o->kill ((float) sr); --act;
    }
    E106::Voice* f = nullptr;
    for (auto& v : voices) if (! v.on) { f = &v; break; }
    if (f == nullptr) for (auto& v : voices) if (f == nullptr || v.age < f->age) f = &v;
    const float freq = 440.f * std::exp2 ((n - 69) / 12.f);
    const float from = (s.port > 0.f && lastF > 0.f) ? lastF : freq;
    lastF = freq;
    f->start (n, vel, from, freq, s, (float) sr, ++ageCtr);
}

void EarthProcessor::noteOff (int n, const E106::Snap& s)
{
    for (auto& v : voices) if (v.on && ! v.released && v.note == n) v.stop (s, (float) sr);
}

void EarthProcessor::renderSeg (AudioBuffer<float>& b, int start, int n, const E106::Snap& s)
{
    if (n <= 0) return;
    float* L = b.getWritePointer (0) + start;
    float* R = b.getNumChannels() > 1 ? b.getWritePointer (1) + start : nullptr;
    const float fsr = (float) sr;
    const float pc = s.port > 0.f ? 1.f - std::exp (-1.f / (s.port / 4.f * fsr)) : 1.f;
    static const float chRate[] = { 0, .5f, .8f, 9.75f }, chDepth[] = { 0, .0018f, .0025f, .0004f };
    const int dsz = (int) dbL.size();

    for (int i = 0; i < n; ++i)
    {
        const float fc = fcS.getNextValue(), res = resS.getNextValue(), m = masterS.getNextValue();
        float cut = 20.f * std::exp (std::log (900.f) * (s.hp ? 1.f - fc : fc));
        cut = std::min (cut, .45f * fsr);
        lfoPh += s.lfoR / fsr; if (lfoPh >= 1.f) lfoPh -= 1.f;
        E106::Ctx c { fsr, std::sin (MathConstants<float>::twoPi * lfoPh), cut, res, pc, bend };

        float in = 0.f;
        for (auto& v : voices) if (v.on) in += v.render (s, c, rnd);

        // Chorus style Juno (I / II / I+II)
        cb[(size_t) cw] = in; cw = (cw + 1) & 4095;
        float yL = in, yR = in;
        if (s.ch > 0)
        {
            chPh += chRate[s.ch] / fsr; if (chPh >= 1.f) chPh -= 1.f;
            const float mod = std::sin (MathConstants<float>::twoPi * chPh), dp = chDepth[s.ch];
            auto rd = [this] (float d) { float r = (float) cw - d; if (r < 0.f) r += 4096.f; const int i0 = (int) r & 4095; const float fr = r - std::floor (r);
                                          return cb[(size_t) i0] * (1.f - fr) + cb[(size_t) ((i0 + 1) & 4095)] * fr; };
            yL += .75f * rd ((.006f + dp * mod) * fsr);
            yR += .75f * rd ((.006f - dp * mod) * fsr);
        }
        // Delay
        dsm += ((float) (s.dlT * sr) - dsm) * .0005f;
        auto rdD = [&] (std::vector<float>& bf, float d) { float r = (float) dw - d; while (r < 0.f) r += (float) dsz; const int i0 = (int) r % dsz; const float fr = r - std::floor (r);
                                                           return bf[(size_t) i0] * (1.f - fr) + bf[(size_t) ((i0 + 1) % dsz)] * fr; };
        const float dl = rdD (dbL, dsm), dr = rdD (dbR, dsm);
        dbL[(size_t) dw] = yL + s.dlF * dl; dbR[(size_t) dw] = yR + s.dlF * dr; dw = (dw + 1) % dsz;
        yL += s.dlM * dl; yR += s.dlM * dr;

        L[i] = std::tanh (yL * m);
        if (R != nullptr) R[i] = std::tanh (yR * m);
    }
}

void EarthProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midi)
{
    ScopedNoDenormals nd;
    buffer.clear();
    const int n = buffer.getNumSamples();
    keyState.processNextMidiBuffer (midi, 0, n, true);
    const auto s = readSnap();
    fcS.setTargetValue (s.fc); resS.setTargetValue (s.res); masterS.setTargetValue (s.master * s.master * .9f);

    int pos = 0;
    for (const auto meta : midi)
    {
        const int t = jlimit (pos, n, meta.samplePosition);
        renderSeg (buffer, pos, t - pos, s); pos = t;
        const auto m = meta.getMessage();
        if (m.isNoteOn()) noteOn (m.getNoteNumber(), m.getFloatVelocity(), s);
        else if (m.isNoteOff()) noteOff (m.getNoteNumber(), s);
        else if (m.isAllNotesOff() || m.isAllSoundOff()) { for (auto& v : voices) if (v.on) v.kill ((float) sr); }
        else if (m.isPitchWheel()) bend = (m.getPitchWheelValue() - 8192) / 8192.f * 200.f;
    }
    renderSeg (buffer, pos, n - pos, s);
}

void EarthProcessor::setCurrentProgram (int i)
{
    cur = jlimit (0, (int) presets.size() - 1, i);
    for (auto& kv : presets[(size_t) cur].v)
        if (auto* p = apvts.getParameter (kv.first))
            p->setValueNotifyingHost (p->convertTo0to1 (kv.second));
}

File EarthProcessor::userFile()
{
    auto d = File::getSpecialLocation (File::userApplicationDataDirectory).getChildFile ("NACHE").getChildFile ("Earth106");
    d.createDirectory();
    return d.getChildFile ("user_presets.xml");
}

void EarthProcessor::loadUser()
{
    if (auto x = XmlDocument::parse (userFile()))
        for (auto* e : x->getChildIterator())
        {
            E106::Preset p; p.name = e->getStringAttribute ("name");
            for (int i = 0; i < e->getNumAttributes(); ++i)
                if (e->getAttributeName (i) != "name") p.v[e->getAttributeName (i)] = (float) e->getAttributeValue (i).getDoubleValue();
            presets.push_back (p);
        }
}

void EarthProcessor::writeUser()
{
    XmlElement root ("Presets");
    for (size_t i = (size_t) nFactory; i < presets.size(); ++i)
    {
        auto* e = root.createNewChildElement ("P");
        e->setAttribute ("name", presets[i].name);
        for (auto& kv : presets[i].v) e->setAttribute (kv.first, (double) kv.second);
    }
    root.writeTo (userFile());
}

void EarthProcessor::saveUserPreset (const String& name)
{
    E106::Preset p; p.name = "USER - " + name;
    for (auto& kv : E106::defaultsMap())
        if (auto* q = apvts.getParameter (kv.first)) p.v[kv.first] = q->convertFrom0to1 (q->getValue());
    presets.push_back (p); cur = (int) presets.size() - 1;
    writeUser();
}

void EarthProcessor::getStateInformation (MemoryBlock& d)
{
    auto st = apvts.copyState(); st.setProperty ("prog", cur, nullptr);
    if (auto x = st.createXml()) copyXmlToBinary (*x, d);
}

void EarthProcessor::setStateInformation (const void* p, int n)
{
    if (auto x = getXmlFromBinary (p, n))
        if (x->hasTagName (apvts.state.getType()))
        {
            auto t = ValueTree::fromXml (*x);
            cur = jlimit (0, (int) presets.size() - 1, (int) t.getProperty ("prog", 0));
            apvts.replaceState (t);
        }
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new EarthProcessor(); }
