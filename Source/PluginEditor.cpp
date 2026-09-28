#include "PluginEditor.h"

EarthEditor::EarthEditor (EarthProcessor& p) : AudioProcessorEditor (&p), content (p)
{
    addAndMakeVisible (content);
    constrainer.setFixedAspectRatio (1200.0 / 620.0);
    constrainer.setSizeLimits (600, 310, 2400, 1240);
    setConstrainer (&constrainer);
    setResizable (true, true);
    setSize (1000, 517);
}

void EarthEditor::resized()
{
    content.setBounds (0, 0, 1200, 620);
    content.setTransform (AffineTransform::scale (getWidth() / 1200.f));
}

juce::AudioProcessorEditor* EarthProcessor::createEditor() { return new EarthEditor (*this); }
