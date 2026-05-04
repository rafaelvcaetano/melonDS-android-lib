#ifndef SCREENSHOTRENDERER_H
#define SCREENSHOTRENDERER_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <GLES3/gl3.h>
#include "GPU.h"
#include "FrameQueue.h"
#include "types.h"
#include "renderer/Renderer.h"

namespace MelonDSAndroid
{

class ScreenshotRenderer {
private:
    static const int SCREENSHOT_WIDTH = 256;
    static const int SCREENSHOT_HEIGHT = 192 * 2;

    u32* screenshotBuffer;
    GLuint frameBuffer;
    GLuint bufferTexture;
    GLuint vao;
    GLuint vbo;
    GLuint screenshotRenderVertexShader;
    GLuint screenshotRenderFragmentShader;
    GLuint screenshotRenderShader;
    GLuint textureUniformLocation;
    GLuint posAttribLocation;
    GLuint texCoordAttribLocation;
    bool screenshotRequested;
    bool stopped;

    void setupFrameBuffer();
    void setupShaders();
    void setupVertexBuffers();
    void notifyScreenshotReady();

    std::mutex screenshotMutex;
    std::condition_variable screenshotCondition;

public:
    ScreenshotRenderer(u32* screenshotBuffer);
    void init();
    void renderScreenshot(GPU* gpu, Renderer renderer, Frame* renderFrame);
    u32* getScreenshot();

    bool takeScreenshot();
    bool isScreenshotPending();

    void cleanup();
};

}

#endif //SCREENSHOTRENDERER_H
