#include "OpenGLContext.h"
#include "MelonLog.h"

const char* OPENGL_CONTEXT_TAG = "OpenGLContext";

void logEglError()
{
    GLint error = eglGetError();
    switch(error)
    {
        case EGL_NOT_INITIALIZED:
            LOG_ERROR(OPENGL_CONTEXT_TAG, "EGL_NOT_INITIALIZED");
            break;
        case EGL_BAD_ACCESS:
            LOG_ERROR(OPENGL_CONTEXT_TAG, "EGL_BAD_ACCESS");
            break;
        case EGL_BAD_ALLOC:
            LOG_ERROR(OPENGL_CONTEXT_TAG, "EGL_BAD_ALLOC");
            break;
        case EGL_BAD_ATTRIBUTE:
            LOG_ERROR(OPENGL_CONTEXT_TAG, "EGL_BAD_ATTRIBUTE");
            break;
        case EGL_BAD_CONTEXT:
            LOG_ERROR(OPENGL_CONTEXT_TAG, "EGL_BAD_CONTEXT");
            break;
        case EGL_BAD_CONFIG:
            LOG_ERROR(OPENGL_CONTEXT_TAG, "EGL_BAD_CONFIG");
            break;
        case EGL_BAD_CURRENT_SURFACE:
            LOG_ERROR(OPENGL_CONTEXT_TAG, "EGL_BAD_CURRENT_SURFACE");
            break;
        case EGL_BAD_DISPLAY:
            LOG_ERROR(OPENGL_CONTEXT_TAG, "EGL_BAD_DISPLAY");
            break;
        case EGL_BAD_SURFACE:
            LOG_ERROR(OPENGL_CONTEXT_TAG, "EGL_BAD_SURFACE");
            break;
        case EGL_BAD_MATCH:
            LOG_ERROR(OPENGL_CONTEXT_TAG, "EGL_BAD_MATCH");
            break;
        case EGL_BAD_PARAMETER:
            LOG_ERROR(OPENGL_CONTEXT_TAG, "EGL_BAD_PARAMETER");
            break;
        case EGL_BAD_NATIVE_PIXMAP:
            LOG_ERROR(OPENGL_CONTEXT_TAG, "EGL_BAD_NATIVE_PIXMAP");
            break;
        case EGL_BAD_NATIVE_WINDOW:
            LOG_ERROR(OPENGL_CONTEXT_TAG, "EGL_BAD_NATIVE_WINDOW");
            break;
        case EGL_CONTEXT_LOST:
            LOG_ERROR(OPENGL_CONTEXT_TAG, "EGL_CONTEXT_LOST");
            break;
    }
}

bool OpenGLContext::InitContext(long sharedGlContext)
{
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY)
    {
        LOG_ERROR(OPENGL_CONTEXT_TAG, "Failed to get display");
        return false;
    }

    int majorVersion, minorVersion;
    if (!eglInitialize(display, &majorVersion, &minorVersion))
    {
        LOG_ERROR(OPENGL_CONTEXT_TAG, "Failed to initialize display");
        return false;
    }
    else
    {
        LOG_DEBUG(OPENGL_CONTEXT_TAG, "Initialised display with version %d.%d", majorVersion, minorVersion);
    }

    EGLint attributes[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE,
    };

    EGLint numConfigs;
    if (!eglChooseConfig(display, attributes, nullptr, 0, &numConfigs))
    {
        LOG_ERROR(OPENGL_CONTEXT_TAG, "Failed to determine the number of configs");
        logEglError();
        return false;
    }

    if (numConfigs <= 0)
    {
        LOG_ERROR(OPENGL_CONTEXT_TAG, "No configs found");
        return false;
    }
    else
    {
        LOG_DEBUG(OPENGL_CONTEXT_TAG, "Found %d configs", numConfigs);
    }

    EGLConfig configs[numConfigs];
    EGLint selectedConfigNumber = -1;

    //eglChooseConfig(display, attributes, &selectedConfig, 1, &selectedConfigNumber);
    eglChooseConfig(display, attributes, configs, numConfigs, &numConfigs);

    for (int i = 0; i < numConfigs; i++)
    {
        int depth, stencil, red, green, blue;

        eglGetConfigAttrib(display, configs[i], EGL_DEPTH_SIZE, &depth);
        eglGetConfigAttrib(display, configs[i], EGL_STENCIL_SIZE, &stencil);
        eglGetConfigAttrib(display, configs[i], EGL_RED_SIZE, &red);
        eglGetConfigAttrib(display, configs[i], EGL_GREEN_SIZE, &green);
        eglGetConfigAttrib(display, configs[i], EGL_BLUE_SIZE, &blue);

        if (depth < 1 || stencil < 0)
            continue;

        if (red == 8 && green == 8 && blue == 8)
        {
            selectedConfigNumber = i;
            break;
        }
    }

    if (selectedConfigNumber < 0)
    {
        LOG_ERROR(OPENGL_CONTEXT_TAG, "Couldn't find matching configuration");
        return false;
    }
    else
    {
        int depth, stencil, red, green, blue;

        eglGetConfigAttrib(display, configs[selectedConfigNumber], EGL_DEPTH_SIZE, &depth);
        eglGetConfigAttrib(display, configs[selectedConfigNumber], EGL_STENCIL_SIZE, &stencil);
        eglGetConfigAttrib(display, configs[selectedConfigNumber], EGL_RED_SIZE, &red);
        eglGetConfigAttrib(display, configs[selectedConfigNumber], EGL_GREEN_SIZE, &green);
        eglGetConfigAttrib(display, configs[selectedConfigNumber], EGL_BLUE_SIZE, &blue);

        LOG_DEBUG(OPENGL_CONTEXT_TAG, "Selected GL contextAttributes (#%d):", selectedConfigNumber);
        LOG_DEBUG(OPENGL_CONTEXT_TAG, "\tRED: %d", red);
        LOG_DEBUG(OPENGL_CONTEXT_TAG, "\tGREEN: %d", green);
        LOG_DEBUG(OPENGL_CONTEXT_TAG, "\tBLUE: %d", blue);
        LOG_DEBUG(OPENGL_CONTEXT_TAG, "\tDEPTH: %d", depth);
        LOG_DEBUG(OPENGL_CONTEXT_TAG, "\tSTENCIL: %d", stencil);
    }

    // Display buffer size doesn't matter since it's not really used
    EGLint surfaceAttributes[] = {
        EGL_WIDTH, 2,
        EGL_HEIGHT, 2,
        EGL_NONE
    };

    surface = eglCreatePbufferSurface(display, configs[selectedConfigNumber], surfaceAttributes);
    if (surface == EGL_NO_SURFACE)
    {
        LOG_ERROR(OPENGL_CONTEXT_TAG, "Failed to create buffer surface");
        logEglError();
        return false;
    }

    int contextAttributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };

    EGLContext sharedContext = reinterpret_cast<EGLContext>(sharedGlContext);
    glContext = eglCreateContext(display, configs[selectedConfigNumber], sharedContext, contextAttributes);
    if (glContext == EGL_NO_CONTEXT)
    {
        LOG_ERROR(OPENGL_CONTEXT_TAG, "Failed to create context");
        logEglError();
        return false;
    }

    return true;
}

bool OpenGLContext::Use()
{
    if (eglMakeCurrent(display, surface, surface, glContext))
    {
        return true;
    }
    else
    {
        LOG_ERROR(OPENGL_CONTEXT_TAG, "Failed to use OpenGL context");
        glGetError();
        return false;
    }
}

void OpenGLContext::Release()
{
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

void OpenGLContext::DeInit()
{
    if (display == EGL_NO_DISPLAY)
        return;

    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    if (glContext != EGL_NO_CONTEXT)
        eglDestroyContext(display, glContext);

    if (surface != EGL_NO_SURFACE)
        eglDestroySurface(display, surface);

    eglTerminate(display);

    display = EGL_NO_DISPLAY;
    surface = EGL_NO_SURFACE;
    glContext = EGL_NO_CONTEXT;
}
