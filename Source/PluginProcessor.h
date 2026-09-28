#pragma once
#include "Params.h"
#include "Presets.h"
#include "Engine.h"
#include <array>

class EarthProcessor : public juce::AudioProcessor
{
public:
    EarthProcessor();
    void prepareToPlay (double sampleRate, int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& l) const override { return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo(); }
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Earth 106"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }
    int getNumPrograms() override { return (int) presets.size(); }
    int getCurrentProgram() override { return cur; }
    void setCurrentProgram (int i) override;
    const juce::String getProgramName (int i) override { return presets[(size_t) juce::jlimit (0, (int) presets.size() - 1, i)].name; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;
    void saveUserPreset (const juce::String& name);

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyState;

private:
    E106::Snap readSnap();
    void noteOn (int n, float vel, const E106::Snap& s);
    void noteOff (int n, const E106::Snap& s);
    void renderSeg (juce::AudioBuffer<float>&, int start, int n, const E106::Snap&);
    static juce::File userFile();
    void loadUser();
    void writeUser();

    std::vector<E106::Preset> presets; int nFactory = 0, cur = 0;
    std::array<E106::Voice, 16> voices;
    double sr = 44100; float lastF = 0, bend = 0, lfoPh = 0, chPh = 0; uint32_t ageCtr = 0;
    juce::Random rnd;
    juce::SmoothedValue<float> fcS, resS, masterS;
    std::vector<float> cb, dbL, dbR; int cw = 0, dw = 0; float dsm = 1000.f;
};
