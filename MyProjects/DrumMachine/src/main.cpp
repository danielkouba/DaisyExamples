#include "daisy_pod.h"
#include "DrumFactory.h"
#include <vector>
#include <memory>

using namespace daisy;

DaisyPod hw;
std::vector<std::unique_ptr<DrumSound>> drumKit;
int currentDrum = 0;
float lastKnob1Value = 0.0f; // Previous value of knob1
float lastKnob2Value = 0.0f; // Previous value of knob2
bool knob1Engaged = false;   // Is knob1 actively changing the value?
bool knob2Engaged = false;   // Is knob2 actively changing the value?

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
    hw.seed.StartLog(true);
    drumKit.push_back(DrumFactory::createDrum("kick", &hw));
    drumKit.push_back(DrumFactory::createDrum("snare", &hw));
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    hw.seed.PrintLine("Drum Machine");
}

void triggerDrum(size_t index) {
    if (index < drumKit.size()) {
        drumKit[index]->trigger();
    }
}

void updateParameters() {
    float knob1Value = hw.knob1.Process();
    float knob2Value = hw.knob2.Process();

    if (!drumKit.empty()) {
        // Handle knob1 (Pitch)
        if (!knob1Engaged && fabs(knob1Value - lastKnob1Value) > 0.05f) {
            knob1Engaged = true; // Activate when user moves past threshold
        }
        if (knob1Engaged) {
            drumKit[currentDrum]->setParameter(ParameterType::Pitch, knob1Value);
            lastKnob1Value = knob1Value; // Update stored value
        }

        // Handle knob2 (Decay)
        if (!knob2Engaged && fabs(knob2Value - lastKnob2Value) > 0.05f) {
            knob2Engaged = true;
        }
        if (knob2Engaged) {
            drumKit[currentDrum]->setParameter(ParameterType::Decay, knob2Value);
            lastKnob2Value = knob2Value;
        }
    }
}

int main(){
    setupDrumMachine();

    while(1){
        hw.ProcessAllControls();
        updateParameters();

        if (hw.button1.RisingEdge()) {
            triggerDrum(0);
            if (currentDrum != 0){
                currentDrum = 0;
                knob1Engaged = false;
                knob2Engaged = false;
            }
            hw.seed.PrintLine("Trigger Kick");
        }

        if (hw.button2.RisingEdge()) {
            triggerDrum(1);
            if (currentDrum != 1){
                currentDrum = 1;
                knob1Engaged = false;
                knob2Engaged = false;
            }
            hw.seed.PrintLine("Trigger Snare");
        }

    }
}