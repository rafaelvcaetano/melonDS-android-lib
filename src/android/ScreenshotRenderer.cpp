#include "ScreenshotRenderer.h"
#include "MelonLog.h"
#include "../GPU.h"

ScreenshotRenderer::ScreenshotRenderer(u32* screenshotBuffer)
{
    this->screenshotBuffer = screenshotBuffer;
    this->currentBuffer = 0;
}

void ScreenshotRenderer::init()
{
    setupFrameBuffers();
    setupShaders();
    setupVertexBuffers();
}

void ScreenshotRenderer::renderScreenshot()
{
    int frontBuffer = GPU::FrontBuffer;
    if (GPU::Renderer == 0)
    {
        memcpy(screenshotBuffer, GPU::Framebuffer[frontBuffer][0], 256 * 192 * 4);
        memcpy(&screenshotBuffer[256 * 192], GPU::Framebuffer[frontBuffer][1], 256 * 192 * 4);
    }
    else
    {
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_BLEND);
        glViewport(0, 0, 256, 192 * 2);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffers[currentBuffer]);
        glUseProgram(screenshotRenderShader);

        // Render to screenshot front buffer
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, GPU::CurGLCompositor->GetOutputTexture(frontBuffer));

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glEnableVertexAttribArray(posAttribLocation);
        glEnableVertexAttribArray(texCoordAttribLocation);
        glVertexAttribPointer(posAttribLocation, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glVertexAttribPointer(texCoordAttribLocation, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glUniform1i(textureUniformLocation, 0);

        glDrawArrays(GL_TRIANGLES, 0, 12);

        // Update screenshotBuffer using back buffer. This is so that we don't have to wait for the GPU to finish rendering the current frame. This means that screenshots in
        // OpenGL have a 1 frame delay, but at least we avoid slowdowns
        int screenshotBackBuffer = currentBuffer ? 0 : 1;
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffers[screenshotBackBuffer]);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels(0, 0, 256, 192 * 2, GL_RGBA, GL_UNSIGNED_BYTE, screenshotBuffer);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        currentBuffer = (currentBuffer + 1) % 2;
    }
}

u32* ScreenshotRenderer::getScreenshot()
{
    return screenshotBuffer;
}

void ScreenshotRenderer::setupFrameBuffers()
{
    glGenFramebuffers(2, frameBuffers);
    glGenTextures(2, bufferTextures);

    for (int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, bufferTextures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 192 * 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferTextures[i], 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ScreenshotRenderer::setupShaders()
{
    const char *vertexShaderSource =
            "#version 100\n"
            "attribute vec2 aTexCoord;\n"
            "attribute vec2 aPosition;\n"
            "varying vec2 vTexCoord;\n"
            "void main() {\n"
            "    gl_Position = vec4(aPosition, 0.0, 1.0);\n"
            "    vTexCoord = aTexCoord;\n"
            "}\n";
    const char *fragmentShaderSource =
            "#version 100\n"
            "precision mediump float;\n"
            "precision mediump sampler2D;\n"
            "varying vec2 vTexCoord;\n"
            "uniform sampler2D sTexture;\n"
            "void main() {\n"
            "    gl_FragColor = texture2D(sTexture, vTexCoord);\n"
            "}\n";

    GLint shaderResult;
    screenshotRenderVertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLint len = strlen(vertexShaderSource);
    glShaderSource(screenshotRenderVertexShader, 1, &vertexShaderSource, &len);
    glCompileShader(screenshotRenderVertexShader);
    glGetShaderiv(screenshotRenderVertexShader, GL_COMPILE_STATUS, &shaderResult);
    if (shaderResult != GL_TRUE)
    {
        glGetShaderiv(screenshotRenderVertexShader, GL_INFO_LOG_LENGTH, &shaderResult);
        if (shaderResult < 1) shaderResult = 1024;
        char* log = new char[shaderResult + 1];
        glGetShaderInfoLog(screenshotRenderVertexShader, shaderResult + 1, NULL, log);
        LOG_ERROR("OpenGL", "Failed to compile vertex shader: %s", log);
        delete[] log;

        glDeleteShader(screenshotRenderVertexShader);

        return;
    }

    screenshotRenderFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    len = strlen(fragmentShaderSource);
    glShaderSource(screenshotRenderFragmentShader, 1, &fragmentShaderSource, &len);
    glCompileShader(screenshotRenderFragmentShader);
    glGetShaderiv(screenshotRenderFragmentShader, GL_COMPILE_STATUS, &shaderResult);
    if (shaderResult != GL_TRUE)
    {
        glGetShaderiv(screenshotRenderFragmentShader, GL_INFO_LOG_LENGTH, &shaderResult);
        if (shaderResult < 1) shaderResult = 1024;
        char* log = new char[shaderResult + 1];
        glGetShaderInfoLog(screenshotRenderFragmentShader, shaderResult + 1, NULL, log);
        LOG_ERROR("OpenGL", "Failed to compile fragment shader: %s", log);
        delete[] log;

        glDeleteShader(screenshotRenderVertexShader);
        glDeleteShader(screenshotRenderFragmentShader);

        return;
    }

    screenshotRenderShader = glCreateProgram();
    glAttachShader(screenshotRenderShader, screenshotRenderVertexShader);
    glAttachShader(screenshotRenderShader, screenshotRenderFragmentShader);
    glLinkProgram(screenshotRenderShader);
    glGetProgramiv(screenshotRenderShader, GL_LINK_STATUS, &shaderResult);
    if (shaderResult != GL_TRUE)
    {
        glGetProgramiv(screenshotRenderShader, GL_INFO_LOG_LENGTH, &shaderResult);
        if (shaderResult < 1) shaderResult = 1024;
        char* log = new char[shaderResult + 1];
        glGetProgramInfoLog(screenshotRenderShader, shaderResult + 1, NULL, log);
        LOG_ERROR("OpenGL", "Failed to link shader program: %s", log);
        delete[] log;

        glDeleteShader(screenshotRenderVertexShader);
        glDeleteShader(screenshotRenderFragmentShader);
        glDeleteProgram(screenshotRenderShader);

        return;
    }

    posAttribLocation = glGetAttribLocation(screenshotRenderShader, "aPosition");
    texCoordAttribLocation = glGetAttribLocation(screenshotRenderShader, "aTexCoord");
    textureUniformLocation = glGetUniformLocation(screenshotRenderShader, "sTexture");
}

void ScreenshotRenderer::setupVertexBuffers()
{
    float margin = 1.0f / 192.0f;
    // Image is vertically flipped
    const float vertices[] = {
        //Position        // UV
        -1.0f,  -1.0f,    0.0f, 0.0f,
        -1.0f,  0.0f,     0.0f, 0.5f - margin,
        1.0f,  0.0f,      1.0f, 0.5f - margin,

        -1.0f,  -1.0f,    0.0f, 0.0f,
        1.0f,  0.0f,      1.0f, 0.5f - margin,
        1.0f, -1.0f,      1.0f, 0.0f,

        -1.0f, 0.0f,      0.0f, 0.5f + margin,
        -1.0f, 1.0f,      0.0f, 1.0f,
        1.0f, 1.0f,       1.0f, 1.0f,

        -1.0f, 0.0f,      0.0f, 0.5f + margin,
        1.0f, 1.0f,       1.0f, 1.0f,
        1.0f, 0.0f,       1.0f, 0.5f + margin,
    };

    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
}

void ScreenshotRenderer::cleanup()
{
    glDeleteShader(screenshotRenderVertexShader);
    glDeleteShader(screenshotRenderFragmentShader);
    glDeleteProgram(screenshotRenderShader);
    glDeleteTextures(2, bufferTextures);
    glDeleteFramebuffers(2, frameBuffers);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}
