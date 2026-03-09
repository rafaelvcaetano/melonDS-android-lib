#ifndef MELONDS_OBOECALLBACK_H
#define MELONDS_OBOECALLBACK_H

#include <oboe/Oboe.h>
#include <fstream>
#include "MelonInstance.h"

class OboeCallback : public oboe::AudioStreamCallback {
private:
    int _volume;
    float audioSampleFrac;

public:
    std::weak_ptr<MelonDSAndroid::MelonInstance> activeInstance;

    OboeCallback(int volume) : OboeCallback(volume, nullptr) { };
    OboeCallback(int volume, std::ostream* recordingStream);
    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *stream, void *audioData, int32_t numFrames) override;
    
private:
    std::ostream* _recordingStream;
    int getNumSamplesOut(int len);
    void audioResample(s16* inbuf, int inlen, s16* outbuf, int outlen, int volume);
};


#endif //MELONDS_OBOECALLBACK_H
