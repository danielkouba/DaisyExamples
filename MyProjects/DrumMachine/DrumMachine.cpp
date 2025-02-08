#include "daisy_pod.h"
#include "daisysp.h"
#include <stdio.h>
#include <string.h>

using namespace daisy;
using namespace daisysp;

DaisyPod hw;
Svf filt;


/////////////////////////////////////////////
// Kick drum class
class KickDrum {
public:
    void Init(float sample_rate) {
        sample_rate_ = sample_rate;
        pitch_env_.Init(sample_rate);
        amp_env_.Init(sample_rate);
        osc_.Init(sample_rate);
        noise_.Init();

        // Configure pitch envelope (fast attack, short decay)
        pitch_env_.SetTime(ADSR_SEG_ATTACK, 0.001f); // Very fast attack
        pitch_env_.SetTime(ADSR_SEG_DECAY, 0.05f);   // Quick pitch drop
        pitch_env_.SetSustainLevel(0.0f);            // No sustain
        pitch_env_.SetTime(ADSR_SEG_RELEASE, 0.01f); // Quick release

        // Configure amplitude envelope
        amp_env_.SetTime(ADSR_SEG_ATTACK, 0.001f);
        amp_env_.SetTime(ADSR_SEG_DECAY, 0.25f); // Short decay for a thumpy kick
        amp_env_.SetSustainLevel(0.0f);         // Ensure it fully stops
        amp_env_.SetTime(ADSR_SEG_RELEASE, 0.01f);

        osc_.SetWaveform(Oscillator::WAVE_SIN); // Sine wave for deep bass
    }

    float Process() {
        if (!amp_env_.IsRunning()) {
            return 0.0f; // Ensure output stops when the envelope is done
        }

        float pitch_mod = pitch_env_.Process(true) * 100.0f; // Pitch sweep effect
        osc_.SetFreq(50.0f + pitch_mod); // Base freq + transient
        float osc_out = osc_.Process();
        float noise_out = noise_.Process() * 0.0f; // Add some noise for attack
        return (osc_out + noise_out) * amp_env_.Process(true); // Apply amplitude envelope
    }

    void Trigger() {
        pitch_env_.Retrigger(true);
        amp_env_.Retrigger(true);
    }

private:
    float sample_rate_;
    Oscillator osc_;
    WhiteNoise noise_;
    Adsr pitch_env_;
    Adsr amp_env_;
};
//End Kick drum class
/////////////////////////////////////////////

/////////////////////////////////////////////
// Snare drum class
class SnareDrum {
public:
    void Init(float sample_rate) {
        sample_rate_ = sample_rate;

        // Initialize oscillators and noise
        bodyOsc_.Init(sample_rate);
        noise_.Init();
        noiseFilter_.Init(sample_rate);

        // Set waveform for body (Triangle for a sharp attack)
        bodyOsc_.SetWaveform(Oscillator::WAVE_TRI);
        bodyOsc_.SetFreq(180.0f); // Tuned snare body frequency

        // Configure noise filter (bandpass to shape the snare rattle)
        noiseFilter_.SetFreq(2000.0f);
        noiseFilter_.SetRes(0.7f);

        // Configure envelopes
        ampEnv_.Init(sample_rate);
        ampEnv_.SetTime(ADSR_SEG_ATTACK, 0.002f);
        ampEnv_.SetTime(ADSR_SEG_DECAY, 0.05f); // Quick decay
        ampEnv_.SetSustainLevel(0.0f);
        ampEnv_.SetTime(ADSR_SEG_RELEASE, 0.01f);

        pitchEnv_.Init(sample_rate);
        pitchEnv_.SetTime(ADSR_SEG_ATTACK, 0.001f);
        pitchEnv_.SetTime(ADSR_SEG_DECAY, 0.02f); // Small pitch drop
        pitchEnv_.SetSustainLevel(0.0f);
        pitchEnv_.SetTime(ADSR_SEG_RELEASE, 0.01f);
    }

    float Process() {
        if (!ampEnv_.IsRunning()) {
            return 0.0f;
        }

        // Snare body (tuned triangle wave with slight pitch envelope)
        float pitch_mod = pitchEnv_.Process(true) * 20.0f;
        bodyOsc_.SetFreq(180.0f + pitch_mod);
        float body = bodyOsc_.Process() * 0.5f;

        // Snare rattle (bandpassed noise)
        // float noise_out = noiseFilter_.Process(noise_.Process()) * 0.8f;
        float noise_out = noise_.Process() * 0.8f; // Add some noise for attack

        // Mix components and apply envelope
        return (body + noise_out) * ampEnv_.Process(true);
    }

    void Trigger() {
        pitchEnv_.Retrigger(true);
        ampEnv_.Retrigger(true);
    }

private:
    float sample_rate_;
    Oscillator bodyOsc_;
    WhiteNoise noise_;
    Svf noiseFilter_;
    Adsr pitchEnv_;
    Adsr ampEnv_;
};

// End of Snare drum class
/////////////////////////////////////////////



KickDrum kick; // Global kick drum instance
SnareDrum snare; // Global snare drum instance


void AudioCallback(AudioHandle::InterleavingInputBuffer in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t size)
{
    float kick_out;
    float snare_out;
    for(size_t i = 0; i < size; i += 2)
    {
        kick_out = kick.Process(); // Process kick drum
        snare_out = snare.Process(); // Process snare drum
        filt.Process(kick_out); // Apply filter to kick drum if needed
        filt.Process(snare_out); // Apply filter to snare drum if needed
        out[i] = out[i + 1] = filt.Low(); // Output kick drum only
    }
}

void HandleMidiMessage(MidiEvent m)
{
    switch(m.type)
    {
        case NoteOn:
        {
            NoteOnEvent p = m.AsNoteOn();
            char buff[512];
            sprintf(buff,
                    "Note Received:\t%d\t%d\t%d\r\n",
                    m.channel,
                    m.data[0],
                    m.data[1]);
            hw.seed.usb_handle.TransmitInternal((uint8_t *)buff, strlen(buff));

            if(p.velocity > 0) // Ignore note-offs (velocity = 0)
            {
                if (p.note == 36) { // C2 (MIDI note 36) triggers the kick drum
                    kick.Trigger();
                }
                else if (p.note == 37) { // C#2 (MIDI note 37) triggers the snare drum
                    snare.Trigger();
                }
            }
            break;
        }
        case ControlChange:
        {
            ControlChangeEvent p = m.AsControlChange();
            char buff[512];
            sprintf(buff,
                    "CC Received:\t%d\t%d\r\n",
                    p.control_number,
                    p.value);
            hw.seed.usb_handle.TransmitInternal((uint8_t *)buff, strlen(buff));

            switch(p.control_number)
            {
                case 1:
                    // Map CC value (0-127) to a frequency range (e.g., 50 Hz to 5000 Hz)
                    filt.SetFreq(50.0f + (p.value / 127.0f) * 4950.0f);
                    break;
                case 2:
                    // Map CC value (0-127) to resonance (0.0 to 1.0)
                    filt.SetRes(p.value / 127.0f);
                    break;
                default: break;
            }
            break;
        }
        default: break;
    }
}

int main(void)
{
    // Initialize hardware
    hw.Init();
    hw.SetAudioBlockSize(48); // More typical block size
    hw.seed.usb_handle.Init(UsbHandle::FS_INTERNAL);
    System::Delay(250); // Allow USB to initialize

    // Initialize synthesis
    float samplerate = hw.AudioSampleRate();
    filt.Init(samplerate);
    kick.Init(samplerate); // Initialize kick drum
    snare.Init(samplerate); // Initialize snare drum

    // Start audio and MIDI
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    hw.midi.StartReceive();

    // Main loop
    while(1)
    {
        hw.midi.Listen();
        while(hw.midi.HasEvents())
        {
            HandleMidiMessage(hw.midi.PopEvent());
        }
    }
}
