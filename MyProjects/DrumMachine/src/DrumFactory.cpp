#include "DrumFactory.h"
#include <memory>

std::unique_ptr<DrumSound> DrumFactory::createDrum(const std::string& type, DaisyPod* hw) {
    if (type == "kick") return std::make_unique<KickDrum>(hw);
    //if (type == "snare") return std::make_unique<SnareDrum>(hw);
    return nullptr;
}