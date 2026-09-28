#pragma once
#include "PluginProcessor.h"
#ifdef HAS_FONT
 #include <BinaryData.h>
#endif
using namespace juce;

static inline Font uiFont (float h, int style = Font::plain, float kern = 0.f)
{
    Font f (FontOptions ("Verdana", h, style)); f.setExtraKerningFactor (kern); return f;
}

//==============================================================================
class ParamDrag : public Component
{
public:
    explicit ParamDrag (RangedAudioParameter& p) : param (p), att (p, [this] (float) { repaint(); }, nullptr) { att.sendInitialUpdate(); }
    void mouseDown (const MouseEvent&) override { att.beginGesture(); start = param.getValue(); }
    void mouseDrag (const MouseEvent& e) override
    {
        const float n = jlimit (0.f, 1.f, start - e.getDistanceFromDragStartY() / (e.mods.isShiftDown() ? 700.f : 200.f));
        att.setValueAsPartOfGesture (param.convertFrom0to1 (n));
    }
    void mouseUp (const MouseEvent&) override { att.endGesture(); }
    void mouseDoubleClick (const MouseEvent&) override { att.setValueAsCompleteGesture (param.convertFrom0to1 (param.getDefaultValue())); }
protected:
    RangedAudioParameter& param;
    ParameterAttachment att;
    float start = 0;
};

//==============================================================================
class Knob : public ParamDrag
{
public:
    Knob (RangedAudioParameter& p, const String& label, bool bigKnob) : ParamDrag (p), lab (label), big (bigKnob)
    { setSize (120, big ? 180 : 104); }

    void paint (Graphics& g) override
    {
        const float cx = 60.f, cy = big ? 90.f : 52.f, rb = big ? 46.f : 22.f, ra = big ? 55.f : 29.f, th = big ? 4.f : 3.f;
        g.setColour (Colour (0xff8fb7f0)); g.setFont (uiFont (big ? 11.f : 10.f, Font::bold, .1f));
        g.drawText (lab, Rectangle<float> (0, cy - (big ? 90.f : 52.f) + (big ? 3.f : 0.f), 120, 18), Justification::centred);
        Path track; track.addCentredArc (cx, cy, ra, ra, 0, degreesToRadians (-135.f), degreesToRadians (135.f), true);
        g.setColour (Colour (0xff16223f)); g.strokePath (track, PathStrokeType (th, PathStrokeType::curved, PathStrokeType::rounded));
        const float n = param.getValue(), a = -135.f + 270.f * n;
        if (n > .003f)
        {
            Path v; v.addCentredArc (cx, cy, ra, ra, 0, degreesToRadians (-135.f), degreesToRadians (a), true);
            g.setColour (Colour (0xff5ecbff)); g.strokePath (v, PathStrokeType (th, PathStrokeType::curved, PathStrokeType::rounded));
        }
        g.setGradientFill (ColourGradient (Colour (0xff2f3b5c), 0, cy - rb, Colour (0xff121a2e), 0, cy + rb, false));
        g.fillEllipse (cx - rb, cy - rb, rb * 2, rb * 2);
        g.setColour (Colour (0xff3a5b8a)); g.drawEllipse (cx - rb, cy - rb, rb * 2, rb * 2, 1.f);
        const float s = std::sin (degreesToRadians (a)), c = -std::cos (degreesToRadians (a));
        g.setColour (Colours::white);
        g.drawLine (cx + s * rb * .3f, cy + c * rb * .3f, cx + s * rb * .75f, cy + c * rb * .75f, big ? 3.f : 2.5f);
        g.setColour (Colour (0xffd7e6ff)); g.setFont (uiFont (big ? 12.f : 11.f));
        g.drawText (param.getCurrentValueAsText(), Rectangle<float> (0, cy + (big ? 66.f : 33.f), 120, 18), Justification::centred);
    }
private:
    String lab; bool big;
};

//==============================================================================
class Globe : public ParamDrag
{
public:
    using ParamDrag::ParamDrag;
    void tick() { rot += param.getValue() * 22.f / 40.f; repaint(); }
    bool hitTest (int x, int y) override { return Point<float> ((float) x, (float) y).getDistanceFrom ({ 180.f, 180.f }) < 176.f; }

    void paint (Graphics& g) override
    {
        static const std::vector<std::vector<std::pair<float, float>>> LAND = {
            { {-17,15},{-10,30},{-5,36},{10,37},{20,32},{32,31},{35,25},{43,12},{51,11},{40,-3},{40,-15},{33,-26},{20,-35},{15,-25},{12,-5},{9,4},{-8,4} },
            { {-9,37},{-9,43},{-2,48},{5,52},{10,55},{20,55},{30,60},{28,70},{15,68},{5,62},{10,58},{0,50},{-5,44} },
            { {30,36},{45,40},{60,38},{70,25},{80,10},{90,22},{100,10},{105,20},{122,30},{122,40},{140,50},{160,60},{180,68},{100,77},{60,70},{40,68},{30,60},{28,45} },
            { {35,30},{48,30},{56,25},{58,20},{45,12},{43,15},{35,28} },
            { {-168,66},{-140,70},{-95,72},{-80,63},{-60,55},{-55,48},{-70,42},{-80,30},{-97,25},{-105,20},{-117,32},{-125,40},{-130,55} },
            { {-80,8},{-60,10},{-50,0},{-35,-7},{-40,-22},{-58,-38},{-68,-55},{-75,-45},{-71,-20},{-81,-5} },
            { {114,-22},{130,-12},{142,-11},{153,-26},{147,-38},{135,-35},{115,-34} },
            { {-55,60},{-20,70},{-25,82},{-60,80} } };

        const float fc = param.getValue(), cx = 180.f, cy = 180.f, tilt = degreesToRadians (20.f);
        { ColourGradient gl (Colour (0x005ecbff), cx, cy, Colour (0x005ecbff), 360.f, cy, true); gl.addColour (.86, Colour (0x665ecbff));
          g.setGradientFill (gl); g.fillEllipse (0, 0, 360, 360); }

        g.saveState();
        Path circ; circ.addEllipse (25, 25, 310, 310); g.reduceClipRegion (circ, AffineTransform());
        { ColourGradient o (Colour (0xff3b86c6), 150.f, 140.f, Colour (0xff08234a), 335.f, 140.f, true); o.addColour (.55, Colour (0xff1d5a9a));
          g.setGradientFill (o); g.fillRect (0, 0, 360, 360); }
        for (auto& poly : LAND)
        {
            Path p; bool any = false, first = true;
            for (auto& pt : poly)
            {
                const float lo = degreesToRadians (pt.first - rot), la = degreesToRadians (pt.second);
                float x = std::cos (la) * std::sin (lo); const float y0 = std::sin (la), z0 = std::cos (la) * std::cos (lo);
                float y = y0 * std::cos (tilt) - z0 * std::sin (tilt); const float z = y0 * std::sin (tilt) + z0 * std::cos (tilt);
                if (z >= 0.f) any = true; else { float h = std::hypot (x, y); if (h < 1e-4f) h = 1.f; x /= h; y /= h; }
                const float px = cx + 155.f * x, py = cy - 155.f * y;
                if (first) { p.startNewSubPath (px, py); first = false; } else p.lineTo (px, py);
            }
            if (! any) continue;
            p.closeSubPath(); g.setColour (Colour (0xff3f8a3d)); g.fillPath (p);
            g.setColour (Colour (0xff245a2a)); g.strokePath (p, PathStrokeType (1.f));
        }
        { ColourGradient vg (Colour (0x00000a1e), cx, cy, Colour (0x8c000a1e), 338.f, cy, true); vg.addColour (.57, Colour (0x00000a1e));
          g.setGradientFill (vg); g.fillRect (0, 0, 360, 360); }
        const float xt = 25.f + 310.f * (1.f - fc);                       // jour / nuit
        g.setColour (Colour::fromFloatRGBA (2 / 255.f, 5 / 255.f, 14 / 255.f, .84f)); g.fillRect (0.f, 0.f, xt, 360.f);
        if (fc > .01f && fc < .99f)
        {
            ColourGradient tg (Colour (0x00d9b27a), 0, 25.f, Colour (0x00d9b27a), 0, 335.f, false); tg.addColour (.5, Colour (0xe6d9b27a));
            g.setGradientFill (tg); g.fillRect (xt - 1.5f, 25.f, 3.f, 310.f);
        }
        g.restoreState();

        g.setColour (Colour (0xb396d7f0)); g.drawEllipse (cx - 156.f, cy - 156.f, 312.f, 312.f, 1.5f);
        const float a = -135.f + 270.f * fc;
        Path track; track.addCentredArc (cx, cy, 170.f, 170.f, 0, degreesToRadians (-135.f), degreesToRadians (135.f), true);
        g.setColour (Colour (0x2e3c64a0)); g.strokePath (track, PathStrokeType (5.f));
        if (fc > .004f)
        {
            Path v; v.addCentredArc (cx, cy, 170.f, 170.f, 0, degreesToRadians (-135.f), degreesToRadians (a), true);
            g.setColour (Colour (0x445ecbff)); g.strokePath (v, PathStrokeType (11.f, PathStrokeType::curved, PathStrokeType::rounded));
            g.setColour (Colour (0xff5ecbff)); g.strokePath (v, PathStrokeType (4.f, PathStrokeType::curved, PathStrokeType::rounded));
        }
        g.setColour (Colours::white);
        g.fillEllipse (cx + 170.f * std::sin (degreesToRadians (a)) - 5.f, cy - 170.f * std::cos (degreesToRadians (a)) - 5.f, 10.f, 10.f);
    }
private:
    float rot = 0;
};

//==============================================================================
class Stars : public Component
{
public:
    Stars() { Random r (7); for (int i = 0; i < 170; ++i) st.push_back ({ r.nextFloat() * 1200.f, r.nextFloat() * 620.f, .5f + r.nextFloat() * 1.1f, .8f + r.nextFloat() * 3.f, r.nextFloat() * 6.28f });
              setInterceptsMouseClicks (false, false); }
    void tick() { t += 1.0 / 40.0; repaint(); }
    void paint (Graphics& g) override
    {
        for (auto& s : st) { g.setColour (Colour (0xffcfe4ff).withAlpha (.15f + .85f * (.5f + .5f * std::sin ((float) t * s.sp + s.ph)))); g.fillEllipse (s.x - s.r, s.y - s.r, s.r * 2, s.r * 2); }
    }
private:
    struct S { float x, y, r, sp, ph; };
    std::vector<S> st; double t = 0;
};

class TitleBar : public Component
{
public:
    std::function<void()> onDouble;
    void paint (Graphics& g) override
    {
        g.setColour (Colour (0xffcfe4ff)); g.setFont (uiFont (22.f, Font::bold, .36f));
        g.drawText ("EARTH 106", 0, 18, 640, 30, Justification::centred);
        g.setColour (Colour (0xff8fb7f0));
       #ifdef HAS_FONT
        static Typeface::Ptr tf = Typeface::createSystemTypefaceFor (BinaryData::NewRockerRegular_ttf, BinaryData::NewRockerRegular_ttfSize);
        g.setFont (Font (FontOptions (tf).withHeight (13.f)));
       #else
        g.setFont (Font (FontOptions ("Algerian", 13.f, Font::plain)));
       #endif
        g.drawText ("by NACHE", 0, 47, 640, 18, Justification::centred);
    }
    void mouseDoubleClick (const MouseEvent&) override { if (onDouble) onDouble(); }
};

class FilterSwitch : public Component
{
public:
    explicit FilterSwitch (RangedAudioParameter& p) : att (p, [this] (float v) { hp = v > .5f; repaint(); }, nullptr) { att.sendInitialUpdate(); }
    void paint (Graphics& g) override
    {
        g.setFont (uiFont (10.f, Font::bold, .3f));
        g.setColour (hp ? Colour (0xffcfe4ff) : Colour (0xff4a6690)); g.drawText ("LOW CUT", Rectangle<float> (0, 0, 75, 20), Justification::centredRight);
        g.setColour (hp ? Colour (0xff4a6690) : Colour (0xffcfe4ff)); g.drawText ("HIGH CUT", Rectangle<float> (125, 0, 75, 20), Justification::centredLeft);
        g.setColour (Colour (0xff16223f)); g.fillRoundedRectangle (83, 2, 34, 16, 8);
        g.setColour (Colour (0xff3a5b8a)); g.drawRoundedRectangle (83.5f, 2.5f, 33, 15, 7.5f, 1.f);
        g.setColour (Colour (0xff8fd4ff)); g.fillEllipse (hp ? 85.f : 101.f, 4.f, 12.f, 12.f);
    }
    void mouseDown (const MouseEvent&) override { att.setValueAsCompleteGesture (hp ? 0.f : 1.f); }
private:
    bool hp = false; ParameterAttachment att;
};

class Tabs : public Component
{
public:
    std::function<void (int)> onChange; int cur = 0;
    void paint (Graphics& g) override
    {
        const char* n[4] = { "GENERAL", "OSC 1", "OSC 2", "FX" };
        const float w = (getWidth() - 12) / 4.f;
        for (int i = 0; i < 4; ++i)
        {
            Rectangle<float> r (i * (w + 4), 0, w, (float) getHeight()); const bool on = i == cur;
            g.setColour (on ? Colour (0xff17305c) : Colour (0xff0c1630)); g.fillRoundedRectangle (r, 4);
            g.setColour (on ? Colour (0xff5ecbff) : Colour (0xff223a66)); g.drawRoundedRectangle (r.reduced (.5f), 4, 1);
            g.setColour (on ? Colour (0xffdff0ff) : Colour (0xff5f82b8)); g.setFont (uiFont (11.f, Font::bold, .15f));
            g.drawText (n[i], r, Justification::centred);
        }
    }
    void mouseDown (const MouseEvent& e) override
    {
        const float w = (getWidth() - 12) / 4.f;
        cur = jlimit (0, 3, (int) (e.position.x / (w + 4))); repaint(); if (onChange) onChange (cur);
    }
};

class About : public Component
{
public:
    About() { addAndMakeVisible (link); addAndMakeVisible (close); close.onClick = [this] { setVisible (false); }; setVisible (false); }
    void paint (Graphics& g) override
    {
        g.fillAll (Colour (0xf0010310)); g.setColour (Colour (0xffd7e6ff));
        g.setFont (uiFont (28.f, Font::bold, .3f)); g.drawText ("EARTH 106", 0, 190, getWidth(), 40, Justification::centred);
        g.setFont (uiFont (15.f)); g.drawText ("Version 1.1", 0, 250, getWidth(), 24, Justification::centred);
        g.drawText ("Date : 28 septembre 2026", 0, 285, getWidth(), 24, Justification::centred);
    }
    void resized() override { link.setBounds (getWidth() / 2 - 120, 340, 240, 24); close.setBounds (getWidth() / 2 - 50, 400, 100, 28); }
private:
    HyperlinkButton link { "Learn more about NACHE", URL ("https://linktr.ee/nachevhs") };
    TextButton close { "Close" };
};

//==============================================================================
class Content : public Component, private Timer
{
public:
    explicit Content (EarthProcessor& p) : proc (p)
    {
        lnf.setColour (PopupMenu::backgroundColourId, Colour (0xff0c1630)); lnf.setColour (PopupMenu::textColourId, Colour (0xffd7e6ff));
        lnf.setColour (PopupMenu::highlightedBackgroundColourId, Colour (0xff17305c));
        lnf.setColour (TextButton::buttonColourId, Colour (0xff101c38)); lnf.setColour (TextButton::textColourOffId, Colour (0xffd7e6ff));
        lnf.setColour (ComboBox::backgroundColourId, Colour (0xff101c38)); lnf.setColour (ComboBox::textColourId, Colour (0xffd7e6ff));
        lnf.setColour (ComboBox::outlineColourId, Colour (0xff2b4776)); lnf.setColour (ComboBox::arrowColourId, Colour (0xff8fb7f0));
        lnf.setColour (TextEditor::backgroundColourId, Colour (0xff101c38)); lnf.setColour (TextEditor::textColourId, Colour (0xffd7e6ff));
        lnf.setColour (TextEditor::outlineColourId, Colour (0xff2b4776)); lnf.setColour (HyperlinkButton::textColourId, Colour (0xff5ecbff));
        setLookAndFeel (&lnf);
        setSize (1200, 620);

        addAndMakeVisible (stars); stars.setBounds (0, 0, 1200, 620);
        addAndMakeVisible (title); title.setBounds (0, 0, 640, 70);
        title.onDouble = [this] { about.setVisible (true); about.toFront (true); };
        addAndMakeVisible (globe); globe.setBounds (138, 140, 360, 360);

        Knob* mk[4] = { &kRes, &kPort, &kPoly, &kMaster };
        const int px[4] = { 83, 555, 83, 555 }, py[4] = { 163, 163, 517, 517 };
        for (int i = 0; i < 4; ++i) { addAndMakeVisible (*mk[i]); mk[i]->setBounds (px[i] - 60, py[i] - 90, 120, 180); }
        addAndMakeVisible (fsw); fsw.setBounds (218, 505, 200, 20);

        addAndMakeVisible (prevB); prevB.setBounds (670, 34, 28, 28);
        addAndMakeVisible (presetBox); presetBox.setBounds (703, 34, 262, 28);
        addAndMakeVisible (nextB); nextB.setBounds (970, 34, 28, 28);
        addAndMakeVisible (nameEd); nameEd.setBounds (1003, 34, 120, 28); nameEd.setTextToShowWhenEmpty ("Nom...", Colour (0xff5f82b8)); nameEd.setInputRestrictions (24);
        addAndMakeVisible (saveB); saveB.setBounds (1128, 34, 42, 28);
        addAndMakeVisible (tabs); tabs.setBounds (670, 70, 500, 28);
        addAndMakeVisible (kbd); kbd.setBounds (670, 500, 510 - 10, 86);
        kbd.setKeyWidth (34.f); kbd.setAvailableRange (48, 72); kbd.setScrollButtonsVisible (false); kbd.setKeyPressBaseOctave (4);
        kbd.setColour (MidiKeyboardComponent::whiteNoteColourId, Colour (0xffdbe6f7)); kbd.setColour (MidiKeyboardComponent::blackNoteColourId, Colour (0xff0a1226));
        kbd.setColour (MidiKeyboardComponent::keyDownOverlayColourId, Colour (0xff5ecbff));
        addChildComponent (about); about.setBounds (0, 0, 1200, 620);

        refreshList();
        presetBox.onChange = [this] { const int id = presetBox.getSelectedId(); if (id > 0) proc.setCurrentProgram (id - 1); };
        prevB.onClick = [this] { step (-1); };  nextB.onClick = [this] { step (1); };
        saveB.onClick = [this]
        {
            auto nm = nameEd.getText().trim(); if (nm.isEmpty()) nm = "Preset " + String (proc.getNumPrograms() - 149);
            proc.saveUserPreset (nm); nameEd.clear(); refreshList();
        };
        tabs.onChange = [this] (int t) { buildTab (t); };
        buildTab (0);
        addMouseListener (this, true);
        startTimerHz (40);
    }
    ~Content() override { setLookAndFeel (nullptr); }

    void paint (Graphics& g) override
    {
        g.setGradientFill (ColourGradient (Colour (0xff02040c), 0, 0, Colour (0xff0a1430), 0, 620, false)); g.fillAll();
        g.setColour (Colour (10, 20, 45).withAlpha (.55f)); g.fillRoundedRectangle (660, 24, 520, 572, 10);
        g.setColour (Colour (0xff1d3157)); g.drawRoundedRectangle (660.5f, 24.5f, 519, 571, 10, 1.f);
        g.setColour (Colour (0xff5f82b8)); g.setFont (uiFont (9.f));
        g.drawText ("Clavier : A W S E D F T G Y H U J K O L P  |  Z / X = octave  |  MIDI supporte", 670, 482, 500, 14, Justification::centred);
    }
    void mouseUp (const MouseEvent& e) override
    {
        if (e.originalComponent != &nameEd && ! nameEd.isParentOf (e.originalComponent)) kbd.grabKeyboardFocus();
    }

private:
    void timerCallback() override
    {
        stars.tick(); globe.tick();
        const int c = proc.getCurrentProgram();
        if (presetBox.getSelectedId() != c + 1 && ! presetBox.isPopupActive()) presetBox.setSelectedId (c + 1, dontSendNotification);
    }
    void step (int d) { const int n = proc.getNumPrograms(); proc.setCurrentProgram ((proc.getCurrentProgram() + d + n) % n); presetBox.setSelectedId (proc.getCurrentProgram() + 1, dontSendNotification); }
    void refreshList()
    {
        presetBox.clear (dontSendNotification);
        for (int i = 0; i < proc.getNumPrograms(); ++i) presetBox.addItem (proc.getProgramName (i), i + 1);
        presetBox.setSelectedId (proc.getCurrentProgram() + 1, dontSendNotification);
    }
    void buildTab (int t)
    {
        std::vector<std::pair<String, String>> d;
        if (t == 0) d = { { "sub", "SUB OSC" }, { "noise", "NOISE" }, { "lfoR", "LFO RATE" }, { "lfoP", "LFO PITCH" }, { "lfoF", "LFO FILTER" }, { "drive", "DRIVE" },
                          { "fEnv", "ENV AMT" }, { "fA", "F ATTACK" }, { "fD", "F DECAY" }, { "fS", "F SUSTAIN" }, { "fR", "F RELEASE" } };
        else if (t == 3) d = { { "ch", "CHORUS" }, { "dlT", "DELAY TIME" }, { "dlF", "FEEDBACK" }, { "dlM", "DELAY MIX" } };
        else { const String k (t); d = { { "w" + k, "WAVE" }, { "o" + k, "OCTAVE" }, { "s" + k, "SEMI" }, { "d" + k, "DETUNE" }, { "l" + k, "LEVEL" }, { "p" + k, "PULSE W" },
                                         { "a" + k, "ATTACK" }, { "dc" + k, "DECAY" }, { "su" + k, "SUSTAIN" }, { "r" + k, "RELEASE" } }; }
        panel.clear();
        int i = 0;
        for (auto& e : d)
        {
            auto* k = panel.add (new Knob (*proc.apvts.getParameter (e.first), e.second, false));
            addAndMakeVisible (k); k->setBounds (670 + (i % 4) * 125 + 2, 108 + (i / 4) * 106, 120, 104); ++i;
        }
        repaint();
    }

    EarthProcessor& proc;
    LookAndFeel_V4 lnf;
    Stars stars; TitleBar title;
    Globe globe { *proc.apvts.getParameter ("fc") };
    Knob kRes { *proc.apvts.getParameter ("res"), "RESONANCE", true }, kPort { *proc.apvts.getParameter ("port"), "PORTAMENTO", true },
         kPoly { *proc.apvts.getParameter ("poly"), "POLYPHONY", true }, kMaster { *proc.apvts.getParameter ("master"), "MASTER", true };
    FilterSwitch fsw { *proc.apvts.getParameter ("hp") };
    Tabs tabs; OwnedArray<Knob> panel;
    TextButton prevB { "<" }, nextB { ">" }, saveB { "SAVE" };
    ComboBox presetBox; TextEditor nameEd;
    MidiKeyboardComponent kbd { proc.keyState, MidiKeyboardComponent::horizontalKeyboard };
    About about;
};

class EarthEditor : public AudioProcessorEditor
{
public:
    explicit EarthEditor (EarthProcessor&);
    void resized() override;
private:
    Content content; ComponentBoundsConstrainer constrainer;
};
