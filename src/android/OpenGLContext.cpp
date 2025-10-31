#include "OpenGLContext.h"
#include "MelonLog.h"
#include "Platform.h"

void logEglError()
{
    GLint error = eglGetError();
    switch(error)
    {
        case EGL_SUCCESS:
            return;
        case EGL_NOT_INITIALIZED:
            melonDS::Platform::Log(melonDS::Platform::LogLevel::Error, "EGL_NOT_INITIALIZED");
            break;
        case EGL_BAD_ACCESS:
            melonDS::Platform::Log(melonDS::Platform::LogLevel::Error, "EGL_BAD_ACCESS");
            break;
        case EGL_BAD_ALLOC:
            melonDS::Platform::Log(melonDS::Platform::LogLevel::Error, "EGL_BAD_ALLOC");
            break;
        case EGL_BAD_ATTRIBUTE:
            melonDS::Platform::Log(melonDS::Platform::LogLevel::Error, "EGL_BAD_ATTRIBUTE");
            break;
        case EGL_BAD_CONTEXT:
            melonDS::Platform::Log(melonDS::Platform::LogLevel::Error, "EGL_BAD_CONTEXT");
            break;
        case EGL_BAD_CONFIG:
            melonDS::Platform::Log(melonDS::Platform::LogLevel::Error, "EGL_BAD_CONFIG");
            break;
        case EGL_BAD_CURRENT_SURFACE:
            melonDS::Platform::Log(melonDS::Platform::LogLevel::Error, "EGL_BAD_CURRENT_SURFACE");
            break;
        case EGL_BAD_DISPLAY:
            melonDS::Platform::Log(melonDS::Platform::LogLevel::Error, "EGL_BAD_DISPLAY");
            break;
        case EGL_BAD_SURFACE:
            melonDS::Platform::Log(melonDS::Platform::LogLevel::Error, "EGL_BAD_SURFACE");
            break;
        case EGL_BAD_MATCH:
            melonDS::Platform::Log(melonDS::Platform::LogLevel::Error, "EGL_BAD_MATCH");
            break;
        case EGL_BAD_PARAMETER:
            melonDS::Platform::Log(melonDS::Platform::LogLevel::Error, "EGL_BAD_PARAMETER");
            break;
        case EGL_BAD_NATIVE_PIXMAP:
            melonDS::Platform::Log(melonDS::Platform::LogLevel::Error, "EGL_BAD_NATIVE_PIXMAP");
            break;
        case EGL_BAD_NATIVE_WINDOW:
            melonDS::Platform::Log(melonDS::Platform::LogLevel::Error, "EGL_BAD_NATIVE_WINDOW");
            break;
        case EGL_CONTEXT_LOST:
            melonDS::Platform::Log(melonDS::Platform::LogLevel::Error, "EGL_CONTEXT_LOST");
            break;
        default:
            melonDS::Platform::Log(melonDS::Platform::LogLevel::Error, "Unknown EGL error: 0x%x", error);
            break;
    }
}

bool OpenGLContext::InitContext(long sharedGlContext)
{
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY)
    {
        melonDS::Platform::Log(melonDS::Platform::LogLevel::Error, "Failed to get display");
        return false;
    }

    int majorVersion, minorVersion;
    if (!eglInitialize(display, &majorVersion, &minorVersion))
    {
        melonDS::Platform::Log(melonDS::Platform::LogLevel::Error, "Failed to initialize display");
        return false;
    }
    else
    {
        melonDS::Platform::Log(melonDS::Platform::LogLevel::Debug, "Initialised display with version %d.%d", majorVersion, minorVersion);
    }

    EGLint attributes[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
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
        melonDS::Platform::Log(melonDS::Platform::LogLevel::Error, "Failed to determine the number of configs");
        logEglError();
        return false;
    }

    if (numConfigs <= 0)
    {
        melonDS::Platform::Log(melonDS::Platform::LogLevel::Error, "No configs found");
        return false;
    }
    else
    {
        melonDS::Platform::Log(melonDS::Platform::LogLevel::Debug, "Found %d configs", numConfigs);
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
        melonDS::Platform::Log(melonDS::Platform::LogLevel::Error, "Couldn't find matching configuration");
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

        melonDS::Platform::Log(melonDS::Platform::LogLevel::Debug, "Selected GL contextAttributes (#%d):", selectedConfigNumber);
        melonDS::Platform::Log(melonDS::Platform::LogLevel::Debug, "\tRED: %d", red);
        melonDS::Platform::Log(melonDS::Platform::LogLevel::Debug, "\tGREEN: %d", green);
        melonDS::Platform::Log(melonDS::Platform::LogLevel::Debug, "\tBLUE: %d", blue);
        melonDS::Platform::Log(melonDS::Platform::LogLevel::Debug, "\tDEPTH: %d", depth);
        melonDS::Platform::Log(melonDS::Platform::LogLevel::Debug, "\tSTENCIL: %d", stencil);
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
        melonDS::Platform::Log(melonDS::Platform::LogLevel::Error, "Failed to create buffer surface");
        logEglError();
        return false;
    }

    int contextAttributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };

    EGLContext sharedContext;
    if (sharedGlContext != 0)
        sharedContext = reinterpret_cast<EGLContext>(sharedGlContext);
    else
        sharedContext = EGL_NO_CONTEXT;

    glContext = eglCreateContext(display, configs[selectedConfigNumber], sharedContext, contextAttributes);
    if (glContext == EGL_NO_CONTEXT)
    {
        melonDS::Platform::Log(melonDS::Platform::LogLevel::Error, "Failed to create context");
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
        melonDS::Platform::Log(melonDS::Platform::LogLevel::Error, "Failed to use OpenGL context");
        logEglError();
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

    if (surface != EGL_NO_SURFACE)
        eglDestroySurface(display, surface);

    if (glContext != EGL_NO_CONTEXT)
        eglDestroyContext(display, glContext);

    eglTerminate(display);

    display = EGL_NO_DISPLAY;
    surface = EGL_NO_SURFACE;
    glContext = EGL_NO_CONTEXT;
}
