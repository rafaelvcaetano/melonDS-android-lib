#ifndef MELONDS_OBOECALLBACK_H
#define MELONDS_OBOECALLBACK_H

#include <oboe/Oboe.h>
#include "MelonInstance.h"

class OboeCallback : public oboe::AudioStreamCallback {
private:
    int _volume;
    float audioSampleFrac;

public:
    std::shared_ptr<MelonDSAndroid::MelonInstance> activeInstance;

    OboeCallback(int volume);
    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *stream, void *audioData, int32_t numFrames) override;
    
private:
    int getNumSamplesOut(int len, int audioFreq);
    void audioResample(s16* inbuf, int inlen, s16* outbuf, int outlen, int volume);
};


#endif //MELONDS_OBOECALLBACK_H
