#ifndef SCREENSHOTRENDERER_H
#define SCREENSHOTRENDERER_H

#include "../types.h"
#include <GLES3/gl3.h>

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
    void renderScreenshot();
    u32* getScreenshot();
    void cleanup();
};

#endif //SCREENSHOTRENDERER_H
