#ifndef DRUM_FACTORY_H
#define DRUM_FACTORY_H

#include "DrumSound.h"
#include "KickDrum.h"
#include "SnareDrum.h"

#include <memory>


class DrumFactory {
public:
    static std::unique_ptr<DrumSound> createDrum(const std::string& type, DaisyPod* hw);
};

#endif