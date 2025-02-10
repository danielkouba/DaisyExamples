#ifndef KICK_DRUM_H
#define KICK_DRUM_H

#include "DrumSound.h"
#include "daisy_pod.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

class KickDrum : public DrumSound {
public:
    KickDrum(DaisyPod* hw);
    void trigger() override;
    void setParameter(ParameterType param, float value) override;
    float Process() override;

private:
    DaisyPod* hardware;
    Oscillator osc;
    AdEnv env;
    float pitch;
    float decay;
};

#endif