#include "daisy_pod.h"
#include "DrumFactory.h"
#include <vector>
#include <memory>

using namespace daisy;

DaisyPod hw;
std::vector<std::unique_ptr<DrumSound>> drumKit;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {
    for (size_t i = 0; i < size; i++) {
        float sample = 0.0f;
        for(auto& drum : drumKit) {
            sample += drum->Process();
        }
        out[0][i] = sample;
        out[1][i] = sample;

    }
}

void setupDrumMachine() {
    hw.Init();
    hw.seed.StartLog();
    drumKit.push_back(DrumFactory::createDrum("kick", &hw));
    drumKit.push_back(DrumFactory::createDrum("snare", &hw));
    hw.seed.StartAudio(AudioCallback);
}

void triggerDrum(int index) {
    if (index < drumKit.size()) {
        drumKit[index]->trigger();
    }
}

void updateParameters() {
    float knob1Value = hw.knob1.Process();
    float knob2Value = hw.knob2.Process();

    if (!drumKit.empty()) {
        drumKit[0]->setParameter(Parameter::Pitch, knob1Value);
        drumKit[0]->setParameter(Parameter::Decay, knob2Value);
    }
}

int main(){
    setupDrumMachine();

    while(1){
        hw.ProcessAllControls();
        updateParameters();

        if (hw.button1.RisingEdge()) {
            triggerDrum(0);
        }

        if (hw.button2.RisingEdge()) {
            triggerDrum(1);
        }

        //hw.Delay(10); //Look into this
    }
}