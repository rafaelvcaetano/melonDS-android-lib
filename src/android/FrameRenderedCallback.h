#ifndef FRAMERENDEREDCALLBACK_H
#define FRAMERENDEREDCALLBACK_H

class FrameRenderedCallback
{
public:
    virtual void onFrameRendered(long syncFence, int textureId) = 0;
};

#endif //FRAMERENDEREDCALLBACK_H
