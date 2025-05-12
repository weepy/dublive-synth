#ifndef ADSR_H
#define ADSR_H

class ADSR {
public:
    ADSR() = default;
    
    void setParameters(float attack, float decay, float sustain, float release) {
        this->attack = attack;
        this->decay = decay;
        this->sustain = sustain;
        this->release = release;
    }
    
    void noteOn() {
        isNoteOn = true;
        position = 0.0f;
        level = 0.0f;
    }
    
    void noteOff() {
        isNoteOn = false;
        noteOffTime = position;
        releaseStartLevel = level;
    }
    
    float process(float deltaTime) {
        position += deltaTime;
        level = calculateLevel(position);
        return level;
    }
    
    bool isActive() const {
        return isNoteOn || level > 0.0001f;
    }
    
private:
    float calculateLevel(float time) {
        float level = 0.0f;
        
        if (isNoteOn) {
            if (time < attack) {
                level = time / attack + 0.00001f;
            } else if (time < attack + decay) {
                float decayPhase = time - attack;
                level = 1.0f - (1.0f - sustain) * (decayPhase / decay) + 0.00001f;
            } else {
                level = sustain;
            }
        } else {
            float releasePhase = time - noteOffTime;
            if (releasePhase < release) {
                level = releaseStartLevel * (1.0f - releasePhase / release);
            } else {
                level = 0.0f;
            }
        }
        
        return level;
    }
    
    float attack = 0.1f;
    float decay = 0.1f;
    float sustain = 0.7f;
    float release = 0.1f;
    float position = 0.0f;
    float level = 0.0f;
    float noteOffTime = 0.0f;
    float releaseStartLevel = 0.0f;
    bool isNoteOn = false;
};

#endif 