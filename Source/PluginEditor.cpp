/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void LookAndFeel::drawRotarySlider(juce::Graphics& g, 
    int x, 
    int y, 
    int width, 
    int height, 
    float sliderPosProportional, 
    float rotaryStartAngle, 
    float rotaryEndAngle, 
    juce::Slider& slider)
{
    using namespace juce;

    auto bounds = Rectangle<float>(x, y, width, height);

    //g.setColour(Colours::black);
    //g.drawEllipse(bounds, 4.f);

    if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
    {
        auto center = bounds.getCentre();
        auto radius = bounds.getWidth() * 0.5f;
        auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = 5.0f;
        auto arcRadius = radius - lineW * 0.5f;
        Path p;

        Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight() * 1.5);

        p.addCentredArc(bounds.getCentreX(),
                        bounds.getCentreY(),
                        arcRadius,
                        arcRadius,
                        0.0f,
                        rotaryStartAngle,
                        rotaryEndAngle,
                        true);
        // p.addRoundedRectangle(r, 2.f);

        jassert(rotaryStartAngle < rotaryEndAngle);

        g.setColour(Colours::black);
        g.strokePath(p, PathStrokeType(lineW, PathStrokeType::curved, PathStrokeType::rounded));
        
        if (slider.isEnabled())
        {
            Path valueArc;
            valueArc.addCentredArc(bounds.getCentreX(),
                                   bounds.getCentreY(),
                                   arcRadius,
                                   arcRadius,
                                   0.0f,
                                   rotaryStartAngle,
                                   toAngle,
                                   true);

            g.setColour(Colour(88u, 224u, 248u));
            g.strokePath(valueArc, PathStrokeType(lineW, PathStrokeType::curved, PathStrokeType::rounded));
            
        }

        Path outline;
        Point<float> outlineEndPoint(bounds.getCentreX() + 1.03f * arcRadius * std::cos(toAngle - MathConstants<float>::halfPi),
                                     bounds.getCentreY() + 1.03f * arcRadius * std::sin(toAngle - MathConstants<float>::halfPi));
        outline.startNewSubPath(center);
        outline.lineTo(outlineEndPoint);
        g.setColour(Colour(143u, 143u, 143u));
        g.strokePath(outline, PathStrokeType(lineW + 5.f, PathStrokeType::curved, PathStrokeType::rounded));

        Path pointer;
        Point<float> endPoint(bounds.getCentreX() + 1.03f * arcRadius * std::cos(toAngle - MathConstants<float>::halfPi),
                              bounds.getCentreY() + 1.03f * arcRadius * std::sin(toAngle - MathConstants<float>::halfPi));
        /*Point<float> stratPoint(bounds.getCentreX() + 0.5f * arcRadius * std::cos(toAngle - MathConstants<float>::halfPi),
                                bounds.getCentreY() + 0.5f * arcRadius * std::sin(toAngle - MathConstants<float>::halfPi));*/
        pointer.startNewSubPath(center);
        pointer.lineTo(endPoint);
        g.setColour(Colours::black);
        g.strokePath(pointer, PathStrokeType(lineW, PathStrokeType::curved, PathStrokeType::rounded));

        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);

        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(Point<float>(bounds.getCentreX(), bounds.getBottom() + 2));

        g.setColour(Colours::transparentBlack);
        g.fillRect(r);

        g.setColour(Colours::black);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }
    
}

void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    using namespace juce;
    
    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;

    auto range = getRange();

    auto sliderBounds = getSliderBounds();

    getLookAndFeel().drawRotarySlider(g,
                                      sliderBounds.getX(),
                                      sliderBounds.getY(),
                                      sliderBounds.getWidth(),
                                      sliderBounds.getHeight(),
                                      jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                                      startAng,
                                      endAng,
                                      *this);

    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5f;

    g.setColour(Colours::black);
    g.setFont(getTextHeight());

    auto numChoices = labels.size();
    for(int i = 0; i<numChoices; ++i)
    {
        Rectangle<float> r;
        auto str = labels[i];
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(Point<float>(center.getX(), sliderBounds.getY() - 12));
        //r.setY(r.getY() + getTextHeight());

        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    // return getLocalBounds();
    auto bounds = getLocalBounds();

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

    size -= 16 * 2 + 5;
    juce::Rectangle<int> r;
    r.setSize(size, size);

    r.setCentre(bounds.getCentreX(), 0);
    r.setY(20);

    return r;
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
        return choiceParam->getCurrentChoiceName();

    juce::String str;
    bool addK = false;

    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
    {
        float val = getValue();

        if (val > 999.f)
        {
            val /= 1000.f; //1001 / 1000 = 1.001
            addK = true;
        }

        str = juce::String(val, (addK ? 1 : 0));
    }
    else
    {
        jassertfalse; //this shouldn't happen!
    }

    if (suffix.isNotEmpty())
    {
        str << " ";
        if (addK)
            str << "k";

        str << suffix;
    }

    return str;
}

ResponseCurveComponent::ResponseCurveComponent(SimpleEQAudioProcessor& p) : audioProcessor(p)
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->addListener(this);
    }
    updateChain();

    startTimerHz(60);
}

ResponseCurveComponent::~ResponseCurveComponent()
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->removeListener(this);
    }
}

void ResponseCurveComponent::parameterValueChanged(int paramterIdenx, float newValue)
{
    parametersChanged.set(true);
}

void ResponseCurveComponent::timerCallback()
{
    if (parametersChanged.compareAndSetBool(false, true))
    {
        DBG("params changed");
        //update the monochain
        updateChain();

        //signal a repaint
        repaint();
    }
}

void ResponseCurveComponent::updateChain()
{
    auto chainSettings = getChainSettings(audioProcessor.apvts);
    auto peakCoeffients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
    auto lowCutCoeffients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
    auto highCutCoeffients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());

    updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoeffients);
    updateCutFilter(monoChain.get<ChainPositions::LowCut>(), lowCutCoeffients, chainSettings.lowCutSlope);
    updateCutFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoeffients, chainSettings.highCutSlope);
}

void ResponseCurveComponent::paint(juce::Graphics& g)
{
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    auto responseArea = getLocalBounds();
    responseArea.setLeft(responseArea.getX() + 10);
    responseArea.setRight(responseArea.getRight() - 10);
    auto w = responseArea.getWidth();

    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    auto& peak = monoChain.get<ChainPositions::Peak>();
    auto& highcut = monoChain.get<ChainPositions::HighCut>();

    auto sampleRate = audioProcessor.getSampleRate();

    std::vector<double> mags;
    mags.resize(w);

    for (int i = 0; i < w; ++i)
    {
        double mag = 1.f;
        auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);//remap

        if (!monoChain.isBypassed<ChainPositions::Peak>())
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);


        if (!monoChain.isBypassed<ChainPositions::LowCut>())
        {
            if (!lowcut.isBypassed<0>())
                mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!lowcut.isBypassed<1>())
                mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!lowcut.isBypassed<2>())
                mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!lowcut.isBypassed<3>())
                mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }

        if (!monoChain.isBypassed<ChainPositions::HighCut>())
        {
            if (!highcut.isBypassed<0>())
                mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!highcut.isBypassed<1>())
                mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!highcut.isBypassed<2>())
                mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!highcut.isBypassed<3>())
                mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }

        mags[i] = Decibels::gainToDecibels(mag);
    }

    Path responseCurve;

    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax](double input)
    {
        return jmap(input, -24.0, 24.0, outputMin, outputMax);
    };

    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));

    for (size_t i = 1; i < mags.size(); ++i)
    {
        responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
    }

    //draw background rectangle
    g.setColour(Colour(26u, 26u, 26u));
    g.fillRoundedRectangle(responseArea.toFloat(), 15.f);
    //draw response curve path
    g.setColour(Colour(88u, 224u, 248u));
    g.strokePath(responseCurve, PathStrokeType(2.f));
}

void ResponseCurveComponent::resized()
{
    using namespace juce;

    background = Image(Image::PixelFormat::RGB, getWidth(), getHeight(), true);
    Graphics g(background);

    Array<float> freqs
    {
        20,30,40,50,60,70,80,90,100,
        200,300,400,500,600,700,800,900,1000,
        2000,3000,4000,5000,600,700,800,900,10000,
        20000
    };
    g.setColour(Colour(49u, 49u, 49u));
    for (auto f : freqs)
    {
        auto normX = mapFromLog10(f, 20.f, 20000.f);
        g.drawVerticalLine(getWidth() * normX, 0.f, getHeight());
    }

    Array<float> gain
    {
        -24, -12, 0, 12, 24
    };
    g.setColour(Colour(49u, 49u, 49u));
    for (auto gDb : gain)
    {
        auto y = jmap(gDb, -24.f, 24.f, float(getHeight()), 0.f);
        g.drawHorizontalLine(y, 0.f, getHeight());
    }
}


//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor(SimpleEQAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    peakFreqSlider(*audioProcessor.apvts.getParameter("Peak Freq"), "Hz"),
    peakGainSlider(*audioProcessor.apvts.getParameter("Peak Gain"), "dB"),
    peakQualitySlider(*audioProcessor.apvts.getParameter("Peak Quality"), ""),
    lowCutFreqSlider(*audioProcessor.apvts.getParameter("LowCut Freq"), "Hz"),
    highCutFreqSlider(*audioProcessor.apvts.getParameter("HighCut Freq"), "Hz"),
    lowCutSlopeSlider(*audioProcessor.apvts.getParameter("LowCut Slope"), "dB/oct"),
    highCutSlopeSlider(*audioProcessor.apvts.getParameter("HighCut Slope"), "dB/oct"),
    
    responseCurveComponent(audioProcessor),
    peakFreqSliderAttachment(audioProcessor.apvts, "Peak Freq", peakFreqSlider),
    peakGainSliderAttachment(audioProcessor.apvts, "Peak Gain", peakGainSlider),
    peakQualitySliderAttachment(audioProcessor.apvts, "Peak Quality", peakQualitySlider),
    lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
    highCutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
    lowCutSlopeSliderAttachment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
    highCutSlopeSliderAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    peakFreqSlider.labels.add("Peak Freq");
    peakGainSlider.labels.add("Peak Gain");
    peakQualitySlider.labels.add("Peak Quality");
    lowCutFreqSlider.labels.add("Low Cut Freq");
    highCutFreqSlider.labels.add("High Cut Freq");
    lowCutSlopeSlider.labels.add("Low Cut Slope");
    highCutSlopeSlider.labels.add("High Cut Slope");

    for(auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }
    
    setSize (700, 400);
}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
}

//==============================================================================
void SimpleEQAudioProcessorEditor::paint(juce::Graphics& g)
{
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll(Colour(143u, 143u, 143u));
}

void SimpleEQAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(10);



    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.7);
    responseCurveComponent.setBounds(responseArea);

    bounds.removeFromTop(10);
    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.2857);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.4);

    lowCutFreqSlider.setBounds(lowCutArea.removeFromLeft(lowCutArea.getWidth() * 0.5));
    lowCutSlopeSlider.setBounds(lowCutArea);

    highCutFreqSlider.setBounds(highCutArea.removeFromRight(highCutArea.getWidth() * 0.5));
    highCutSlopeSlider.setBounds(highCutArea);

    peakFreqSlider.setBounds(bounds.removeFromLeft(bounds.getWidth() * 0.33));
    peakGainSlider.setBounds(bounds.removeFromLeft(bounds.getWidth() * 0.5));
    peakQualitySlider.setBounds(bounds);
}

std::vector<juce::Component*> SimpleEQAudioProcessorEditor::getComps()
{
    return
    {
        &peakFreqSlider,
        &peakGainSlider,
        &peakQualitySlider,
        &lowCutFreqSlider,
        &highCutFreqSlider,
        &lowCutSlopeSlider,
        &highCutSlopeSlider,
        &responseCurveComponent
    };
}