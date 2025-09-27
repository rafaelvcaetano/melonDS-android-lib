#include "FrameQueue.h"

FrameQueue::FrameQueue()
{
    for (auto& frame : frames)
    {
        freeQueue.push(&frame);
    }
}

Frame* FrameQueue::getRenderFrame()
{
    std::unique_lock lock(frameLock);

    Frame* frame;

    // If there are no free frames, use oldest present frame
    if (freeQueue.empty())
    {
        frame = presentQueue.back();
        presentQueue.pop_back();
    }
    else
    {
        frame = freeQueue.front();
        freeQueue.pop();
    }

    return frame;
}

Frame* FrameQueue::getPresentFrame()
{
    std::unique_lock lock(frameLock);

    if (presentQueue.empty()) {
        return previousFrame;
    }

    if (previousFrame)
    {
        freeQueue.push(previousFrame);
        previousFrame = nullptr;
    }

    Frame* frame = presentQueue.front();
    presentQueue.pop_front();

    for (auto f : presentQueue)
    {
        freeQueue.push(f);
    }

    presentQueue.clear();
    previousFrame = frame;
    return frame;
}

void FrameQueue::validateRenderFrame(Frame* frame, int requiredWidth, int requiredHeight)
{
    if (frame->width != requiredHeight && frame->height != requiredHeight)
    {
        // Update frame texture to have the required size
        if (!frame->frameTexture)
        {
            glGenTextures(1, &frame->frameTexture);
            glBindTexture(GL_TEXTURE_2D, frame->frameTexture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, frame->frameTexture);
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, requiredWidth, requiredHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
        frame->width = requiredWidth;
        frame->height = requiredHeight;
    }
}

void FrameQueue::pushRenderedFrame(Frame* frame)
{
    std::unique_lock lock(frameLock);
    presentQueue.push_front(frame);
}

void FrameQueue::clear()
{
    std::unique_lock lock(frameLock);

    for (auto f : presentQueue)
    {
        freeQueue.push(f);
    }

    presentQueue.clear();
    previousFrame = nullptr;

    for (auto& frame : frames) {
        glDeleteTextures(1, &frame.frameTexture);
        glDeleteSync(frame.renderFence);
        glDeleteSync(frame.presentFence);
        frame.frameTexture = 0;
        frame.width = 0;
        frame.height = 0;
        frame.renderFence = 0;
        frame.presentFence = 0;
    }
}
