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
    u32* screenshotBuffer;
    GLuint frameBuffers[2];
    GLuint bufferTextures[2];
    GLuint vao;
    GLuint vbo;
    GLuint screenshotRenderVertexShader;
    GLuint screenshotRenderFragmentShader;
    GLuint screenshotRenderShader;
    GLuint textureUniformLocation;
    GLuint posAttribLocation;
    GLuint texCoordAttribLocation;
    int currentBuffer;

    void setupFrameBuffers();
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
