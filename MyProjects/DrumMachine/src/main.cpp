#include "daisy_pod.h"
#include "DrumFactory.h"
#include <vector>
#include <memory>

using namespace daisy;

DaisyPod hw;
std::vector<std::unique_ptr<DrumSound>> drumKit;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {
    for (size_t i = 0; i < size; i++) {
        out[0][i] = 0.1f; // Left channel (constant low volume signal)
        out[1][i] = 0.1f; // Right channel
    }
}

// void setupDrumMachine() {
//     hw.Init();
//     hw.seed.StartLog(true);
//     drumKit.push_back(DrumFactory::createDrum("kick", &hw));
//     // drumKit.push_back(DrumFactory::createDrum("snare", &hw));
//     hw.seed.StartAudio(AudioCallback);
//     hw.seed.PrintLine("Drum Machine");
// }

void triggerDrum(size_t index) {
    if (index < drumKit.size()) {
        drumKit[index]->trigger();
    }
}

void updateParameters() {
    float knob1Value = hw.knob1.Process();
    float knob2Value = hw.knob2.Process();
    
    if (!drumKit.empty()) {
        drumKit[0]->setParameter(ParameterType::Pitch, knob1Value);
        drumKit[0]->setParameter(ParameterType::Decay, knob2Value);
    }
}

int main(){
    // setupDrumMachine();

    hw.Init();
    hw.seed.StartLog(true);
    drumKit.push_back(DrumFactory::createDrum("kick", &hw));
    // drumKit.push_back(DrumFactory::createDrum("snare", &hw));
    hw.seed.StartAudio(AudioCallback);
    hw.seed.PrintLine("Drum Machine");

    while(1){
        hw.ProcessAllControls();
        updateParameters();

        if (hw.button1.RisingEdge()) {
            triggerDrum(0);
            hw.seed.PrintLine("Trigger Kick");
        }

        if (hw.button2.RisingEdge()) {
            triggerDrum(1);
            hw.seed.PrintLine("Trigger Snare");
        }

        //hw.Delay(10); //Look into this
    }
}