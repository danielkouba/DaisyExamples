#include "KickDrum.h"

KickDrum::KickDrum(DaisyPod* hw) : hardware(hw), pitch(440.0f), decay(0.5f) {
    // Initialize the oscillator
    osc.Init(hw->AudioSampleRate());
    osc.SetWaveform(osc.WAVE_SIN);
    osc.SetFreq(pitch);

    // Initialize the envelope
    env.Init(hw->AudioSampleRate());
    env.SetTime(ADENV_SEG_ATTACK, 0.01f);
    env.SetTime(ADENV_SEG_DECAY, decay);
    env.SetMax(1.0f);
    env.SetMin(0.0f);
}

void KickDrum::trigger() {
    env.Trigger();
}

void KickDrum::setParameter(ParameterType param, float value) {
    switch (param) {
        case ParameterType::Pitch:
            pitch = value * 1000.0f; // Map to a reasonable range
            osc.SetFreq(pitch);
            break;
        case ParameterType::Decay:
            decay = value;
            env.SetTime(ADENV_SEG_DECAY, decay);
            break;
        default:
            break;
    }
}

float KickDrum::Process() {
    return osc.Process() * env.Process() * 1.5f;
}