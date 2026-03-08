#include "OboeCallback.h"
#include "types.h"
#include "Platform.h"
#include "SPU.h"

using namespace melonDS;

#define INTERNAL_FRAME_RATE 59.8260982880808f

OboeCallback::OboeCallback(int volume, std::ostream* recordingStream) : _volume(volume), _recordingStream(recordingStream) {
    audioSampleFrac = 0;
}

oboe::DataCallbackResult
OboeCallback::onAudioReady(oboe::AudioStream *stream, void *audioData, int32_t numFrames) {
    if (!activeInstance)
    {
        memset(audioData, 0, numFrames * sizeof(u16) * 2);
        return oboe::DataCallbackResult::Continue;
    }

    int len = numFrames;

    double skew = std::clamp(60.0 / INTERNAL_FRAME_RATE, 0.995, 1.005);
    activeInstance->setAudioOutputSkew(skew);

    int len_in = getNumSamplesOut(len);
    if (len_in > numFrames) len_in = numFrames;

    int num_in = activeInstance->readAudioOutput((s16*) audioData, len_in);

    //s16 bufferIn[512 * 2];
    //audioResample(bufferIn, num_in, (s16*) audioData, len_in, _volume);

    if (num_in < 1)
    {
        memset(audioData, 0, len * sizeof(s16) * 2);
        return oboe::DataCallbackResult::Continue;
    }

    if (_volume < 256)
    {
        s16* samples = (s16*) audioData;
        for (int i = 0; i < num_in * 2; i++)
            samples[i] = ((s32) samples[i] * _volume) >> 8;
    }

    int margin = 6;
    if (num_in < len_in - margin)
    {
        int last = num_in - 1;

        for (int i = num_in; i < len_in - margin; i++)
            ((u32*)audioData)[i] = ((u32*)audioData)[last];
    }

    if (_recordingStream)
        _recordingStream->write((char*) audioData, numFrames * sizeof(s16) * 2);

    return oboe::DataCallbackResult::Continue;
}

int OboeCallback::getNumSamplesOut(int len)
{
    // TODO: adjust to game speed
    float f_len_in = len /* * (curFPS/60.0)*/;
    f_len_in += audioSampleFrac;
    int len_in = (int) floor(f_len_in);
    audioSampleFrac = f_len_in - len_in;

    return len_in;
}

void OboeCallback::audioResample(s16* inbuf, int inlen, s16* outbuf, int outlen, int volume)
{
    float res_incr = inlen / (float)outlen;
    float res_timer = -0.5;
    int res_pos = 0;

    for (int i = 0; i < outlen; i++)
    {
        s16 l1 = inbuf[res_pos * 2];
        s16 l2 = inbuf[res_pos * 2 + 2];
        s16 r1 = inbuf[res_pos * 2 + 1];
        s16 r2 = inbuf[res_pos * 2 + 3];

        float l = (float) l1 + ((l2 - l1) * res_timer);
        float r = (float) r1 + ((r2 - r1) * res_timer);

        outbuf[i*2  ] = (s16) (((s32) round(l) * volume) >> 8);
        outbuf[i*2+1] = (s16) (((s32) round(r) * volume) >> 8);

        res_timer += res_incr;
        while (res_timer >= 1.0)
        {
            res_timer -= 1.0;
            res_pos++;
        }
    }
}