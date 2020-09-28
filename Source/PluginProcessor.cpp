/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SirenePlugAudioProcessor::SirenePlugAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    startTimer(1);
}

SirenePlugAudioProcessor::~SirenePlugAudioProcessor()
{
}

//==============================================================================
const juce::String SirenePlugAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SirenePlugAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SirenePlugAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SirenePlugAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SirenePlugAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SirenePlugAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SirenePlugAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SirenePlugAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SirenePlugAudioProcessor::getProgramName (int index)
{
    return {};
}

void SirenePlugAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SirenePlugAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void SirenePlugAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SirenePlugAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SirenePlugAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    /* Pat: original code of the audio plug in template
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
     */
    
    buffer.clear();
 
    juce::MidiBuffer processedMidi;
    int time;
    juce::MidiMessage m;
 
    for (juce::MidiBuffer::Iterator i (midiMessages); i.getNextEvent (m, time);)
    {
        if (m.isNoteOn())
        {
            juce::uint8 newVel = (juce::uint8)noteOnVel;
            m = juce::MidiMessage::noteOn(m.getChannel(), m.getNoteNumber(), newVel);
            midiMessageIntArray = getIntFromMidiMessage(m.getRawData(), m.getRawDataSize());
            std::cout << "Message reçu ----------------------------------------------------------------------" << std::endl;
            std::cout << "Message: " << midiMessageIntArray[0] << "-" << midiMessageIntArray[1] << "-" << midiMessageIntArray[2] << "\n";

        }
        else if (m.isNoteOff())
        {
        }
        else if (m.isAftertouch())
        {
        }
        else if (m.isPitchWheel())
        {
        }
 
        processedMidi.addEvent (m, time);
    }
 
    midiMessages.swapWith (processedMidi);
}

//==============================================================================
bool SirenePlugAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SirenePlugAudioProcessor::createEditor()
{
    return new SirenePlugAudioProcessorEditor (*this);
}

//==============================================================================
void SirenePlugAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SirenePlugAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}



int* SirenePlugAudioProcessor::getIntFromMidiMessage(const void * data, int size)
// From a midi message and its size, output the midi message as an array of 3 integers
{
    static int arr[3];
    unsigned int x;

    juce::String hexaMessage =  juce::String::toHexString (data, size); // convert message to hexadecimal string

    juce::String value;
    int begin, end;
    // loop to split the string in 3 and convert each part in integer
    for (int i = 0; i < 3; ++i)
    {
        std::stringstream ss;
        begin = i*3;
        end = begin + 2;
        value = hexaMessage.substring(begin, end);
        ss << std::hex << value;
        ss >> x;
        arr[i] = static_cast<int>(x);
    }
    return arr;
}

void SirenePlugAudioProcessor::timerCallback()
{
    int a = 1;
}



//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SirenePlugAudioProcessor();
}
