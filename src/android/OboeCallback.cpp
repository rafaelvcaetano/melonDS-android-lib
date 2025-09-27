#include "OboeCallback.h"
#include "types.h"
#include "../SPU.h"

using namespace melonDS;

OboeCallback::OboeCallback(int volume) : _volume(volume) {
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

    // resample incoming audio to match the output sample rate

    int len_in = getNumSamplesOut(len, (int) stream->getSampleRate());
    s16 buf_in[1024 * 2];
    int num_in;

    // TODO: audio sync
    num_in = activeInstance->readAudioOutput(buf_in, len_in);

    if (num_in < 1)
    {
        memset(audioData, 0, len * sizeof(s16) * 2);
        return oboe::DataCallbackResult::Continue;
    }

    int margin = 6;
    if (num_in < len_in - margin)
    {
        int last = num_in-1;

        for (int i = num_in; i < len_in - margin; i++)
            ((u32*) buf_in)[i] = ((u32*) buf_in)[last];

        num_in = len_in - margin;
    }

    audioResample(buf_in, num_in, (s16*) audioData, len, _volume);
    return oboe::DataCallbackResult::Continue;
}

int OboeCallback::getNumSamplesOut(int len, int audioFreq)
{
    // TODO: adjust to game speed
    float f_len_in = (len * 32823.6328125 /* * (curFPS/60.0)*/) / (float)audioFreq;
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