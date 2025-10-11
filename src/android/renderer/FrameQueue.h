#ifndef FRAMEQUEUE_H
#define FRAMEQUEUE_H

#include <array>
#include <queue>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include "OpenGLContext.h"
#include "types.h"

using namespace melonDS;

// 9 frames should allow the emulator to run up 8x speed. This includes 8 frames ready to present, plus one frame currently being rendered to.
constexpr std::size_t FRAME_QUEUE_SIZE = 9;

struct Frame {
    GLuint frameTexture{};
    u32 width{};
    u32 height{};
    EGLSyncKHR renderFence{};
    EGLSyncKHR presentFence{};
};

class FrameQueue
{
public:
    FrameQueue();
    Frame* getRenderFrame();
    Frame* getPresentFrame();
    void validateRenderFrame(Frame* frame, int requiredWidth, int requiredHeight);
    void pushRenderedFrame(Frame* frame);
    void discardRenderedFrame(Frame* frame);
    void clear();

private:
    std::mutex frameLock;
    std::array<Frame, FRAME_QUEUE_SIZE> frames{};
    std::queue<Frame*> freeQueue{};
    std::deque<Frame*> presentQueue{};
    Frame* previousFrame = nullptr;
};

#endif //FRAMEQUEUE_H
