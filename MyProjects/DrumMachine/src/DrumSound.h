#ifndef DRUM_SOUND_H
#define DRUM_SOUND_H

enum class ParameterType {
    Pitch,
    Decay
};

class DrumSound {
    public:
        virtual void trigger() = 0;
        virtual void setParameter(ParameterType param, float value) = 0;
        virtual float Process() = 0;
        virtual ~DrumSound() {}
};

#endif