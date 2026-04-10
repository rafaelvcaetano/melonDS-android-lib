#ifndef SCREENSHOTRENDERER_H
#define SCREENSHOTRENDERER_H

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
    static const int SCREENSHOT_BUFFER_SIZE = SCREENSHOT_WIDTH * SCREENSHOT_HEIGHT * 4;

    u32* screenshotBuffer;
    GLuint frameBuffer;
    GLuint bufferTexture;
    GLuint pbos[2];
    GLuint vao;
    GLuint vbo;
    GLuint screenshotRenderVertexShader;
    GLuint screenshotRenderFragmentShader;
    GLuint screenshotRenderShader;
    GLuint textureUniformLocation;
    GLuint posAttribLocation;
    GLuint texCoordAttribLocation;
    int currentReadPbo;
    bool firstFrameRendered;

    void setupFrameBuffer();
    void setupPbos();
    void setupShaders();
    void setupVertexBuffers();

public:
    ScreenshotRenderer(u32* screenshotBuffer);
    void init();
    void renderScreenshot(GPU* gpu, Renderer renderer, Frame* renderFrame);
    u32* getScreenshot();
    void cleanup();
};

}

#endif //SCREENSHOTRENDERER_H
