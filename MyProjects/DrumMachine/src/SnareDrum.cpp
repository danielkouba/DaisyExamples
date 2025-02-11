#include "SnareDrum.h"

SnareDrum::SnareDrum(DaisyPod* hw) : hardware(hw), noise_amt(0.7f), decay(0.3f) {
    noise.Init();
    
    // Initialize filter for shaping snare sound
    filter.Init(hw->seed.AudioSampleRate());
    filter.SetFreq(4000.0f);
    filter.SetRes(0.7f);

    // Initialize envelope (attack-decay only, no sustain/release)
    env.Init(hw->seed.AudioSampleRate());
    env.SetTime(ADENV_SEG_ATTACK, 0.01f);  // Short attack
    env.SetTime(ADENV_SEG_DECAY, decay);   // Decay time (0.1 - 0.5s)
}

void SnareDrum::trigger() {
    hardware->seed.PrintLine("Snare Drum Triggered! Noise Amt: %.2f, Decay: %.2f", noise_amt, decay);
    env.Trigger();
}

void SnareDrum::setParameter(ParameterType param, float value) {
    switch (param) {
        case ParameterType::Pitch: // For snare, this controls noise intensity
            noise_amt = value;
            hardware->seed.PrintLine("Snare Noise Set to: %.2f", noise_amt);
            break;
        case ParameterType::Decay:
            decay = 0.1f + (value * 0.4f); // Decay range: 0.1s - 0.5s
            env.SetTime(ADSR_SEG_DECAY, decay);
            hardware->seed.PrintLine("Snare Decay Set to: %.2f", decay);
            break;
    }
}

float SnareDrum::Process() {
    float env_out = env.Process(); // Get envelope value (amplitude)

    if (env_out > 0.0f) { // Only process sound if envelope is active
        float noise_sample = noise.Process() * noise_amt;  // Generate noise
        filter.Process(noise_sample * env_out);  // Apply filter (but no return value)

        float filtered_sample = filter.Low(); // ✅ Get the actual filtered output
        return filtered_sample;
    } else {
        return 0.0f; // No sound when envelope is inactive
    }
}
