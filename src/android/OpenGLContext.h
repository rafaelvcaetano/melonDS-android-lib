#ifndef OPENGLCONTEXT_H
#define OPENGLCONTEXT_H

#include <GLES3/gl32.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

class OpenGLContext
{
public:
    bool InitContext(long sharedGlContext);
    bool Use();
    void Release();
    void DeInit();
    [[nodiscard]] EGLContext GetContext() const { return glContext; }

private:
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLSurface surface = EGL_NO_SURFACE;
    EGLContext glContext = EGL_NO_CONTEXT;
};

#endif //OPENGLCONTEXT_H
