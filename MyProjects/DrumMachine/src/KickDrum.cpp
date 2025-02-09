#include "KickDrum.h"

KickDrum::KickDrum(DaisyPod* hw) : hardware(hw), pitch(50.0f), decay(0.5f) {
    osc.Init(hw->seed.AudioSampleRate());
    osc.SetWaveform(Oscillato::WAVE_SIN);
    env.Init(hw->seed.AudioSampleRate());
    env.SetTime(ADSR_SEG_ATTACK, 0.01f);
    env.SetTime(ADSR_SEG_DECAY, decay);
    env.SetSustainLevel(0.0f);
    env.SetTime(ADSR_SEG_RELEASE, 0.0f);
}

void KickDrum::trigger() {
    hardware->seed.PrintLine("Kick Drum Triggered! Pitch: %.2f Hz, Decay: %.2f", pitch, decay);
    env.Trigger();
}

void KickDrum::setParameter(ParameterType param, float value) {
    switch (param) {
        case ParameterType::Pitch:
            pitch = 40.0f + (value*80.0f);
            hardware->seed.PrintLine("Kick Pitch Set to: %.2f Hz", pitch);
            break;
        case ParameterType::Decay:
            decay = 0.1f + (value * 0.4f);
            env.SetTime(ADSR_SEG_DECAY, decay);
            hardware->seed.PrintLine("Kick Decay Set to: %.2f", decay);
            break;
    }
}

float KickDrum::Process() {
    float env_out = env.Process();
    osc.SetFreq(pitch * (1.0f + env_out * 0.5f));
    return osc.Process() * env_out;
}