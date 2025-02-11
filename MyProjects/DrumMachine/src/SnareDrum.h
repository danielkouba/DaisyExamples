#ifndef SNARE_DRUM_H
#define SNARE_DRUM_H

#include "DrumSound.h"
#include "daisy_pod.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

class SnareDrum : public DrumSound {
public:
    SnareDrum(DaisyPod* hw);
    void trigger() override;
    void setParameter(ParameterType param, float value) override;
    float Process() override;

private:
    DaisyPod* hardware;
    WhiteNoise noise;  // White noise generator for snare sound
    Svf filter;        // State-variable filter for shaping the snare
    AdEnv env;         // Envelope for shaping the snare hit
    float noise_amt;   // Controls the level of noise in the snare
    float decay;       // Decay time of the snare drum
};

#endif
