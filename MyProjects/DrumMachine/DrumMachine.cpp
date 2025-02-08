#include "daisy_pod.h"
#include "daisysp.h"
#include <stdio.h>
#include <string.h>

using namespace daisy;
using namespace daisysp;

DaisyPod hw;
Svf filt;


class Drum {
    public:
        void Init(float sample_rate, float base_freq, bool use_noise = false) {
            sample_rate_ = sample_rate;
            base_freq_ = base_freq;
            use_noise_ = use_noise;
    
            osc_.Init(sample_rate);
            noise_.Init();
            filter_.Init(sample_rate);
    
            // Default to sine wave for low-end drums
            osc_.SetWaveform(Oscillator::WAVE_SIN);
            osc_.SetFreq(base_freq);
    
            // Envelope settings
            amp_env_.Init(sample_rate);
            amp_env_.SetTime(ADSR_SEG_ATTACK, 0.001f);
            amp_env_.SetTime(ADSR_SEG_DECAY, 0.25f);
            amp_env_.SetSustainLevel(0.0f);
            amp_env_.SetTime(ADSR_SEG_RELEASE, 0.01f);
    
            pitch_env_.Init(sample_rate);
            pitch_env_.SetTime(ADSR_SEG_ATTACK, 0.001f);
            pitch_env_.SetTime(ADSR_SEG_DECAY, 0.05f);
            pitch_env_.SetSustainLevel(0.0f);
            pitch_env_.SetTime(ADSR_SEG_RELEASE, 0.01f);
    
            // Default filter settings for noise-based drums
            filter_.SetFreq(2000.0f);
            filter_.SetRes(0.7f);
        }
    
        float Process() {
            if (!amp_env_.IsRunning()) {
                return 0.0f;
            }
    
            float pitch_mod = pitch_env_.Process(true) * 100.0f;
            osc_.SetFreq(base_freq_ + pitch_mod);
            float osc_out = osc_.Process() * 0.8f;
    
            // float noise_sample = noise_.Process(); // Get noise sample
            // float noise_out = use_noise_ ? filter_.Process(noise_sample) * 0.4f : 0.0f;            
            // return (osc_out + noise_out) * amp_env_.Process(true);
    
            return (osc_out) * amp_env_.Process(true);
        }
    
        void Trigger() {
            pitch_env_.Retrigger(true);
            amp_env_.Retrigger(true);
        }
    
        void SetTone(float freq) { base_freq_ = freq; }
        void SetDecay(float decay) { amp_env_.SetTime(ADSR_SEG_DECAY, decay); }
        void SetFilter(float freq, float res) { filter_.SetFreq(freq); filter_.SetRes(res); }
    
    private:
        float sample_rate_;
        float base_freq_;
        bool use_noise_;
    
        Oscillator osc_;
        WhiteNoise noise_;
        Svf filter_;
        Adsr pitch_env_;
        Adsr amp_env_;
    };
    

Drum kick, snare, closedhat; // Global snare drum instance

void InitDrums(float sample_rate) {
    kick.Init(sample_rate, 50.0f, false);  // Kick: Deep sine wave
    snare.Init(sample_rate, 180.0f, true); // Snare: Noise + triangle wave
    closedhat.Init(sample_rate, 3000.0f, true); // Hi-hat: Mostly noise, high frequency
}

void AudioCallback(AudioHandle::InterleavingInputBuffer in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t size)
{
    float kick_out;
    float snare_out;
    float closedhat_out;

    for(size_t i = 0; i < size; i += 2)
    {
        kick_out = kick.Process(); // Process kick drum
        snare_out = snare.Process(); // Process snare drum
        closedhat_out = closedhat.Process(); // Process closed hat
        filt.Process(kick_out); // Apply filter to kick drum if needed
        filt.Process(snare_out); // Apply filter to snare drum if needed
        filt.Process(closedhat_out); // Apply filter to snare drum if needed
        out[i] = out[i + 1] = filt.Low(); // Output
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
                switch (p.note) {
                    case 36: kick.Trigger(); break; // Kick (C2)
                    case 37: snare.Trigger(); break; // Snare (C#2)
                    case 38: closedhat.Trigger(); break; // Hi-hat (D2)
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
    InitDrums(samplerate);

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
