project(melonDS-lib)

set(ANDROID_SOURCES
        MelonAudioStreamErrorCallback.cpp
        MelonDS.cpp
        OboeCallback.cpp
        MicInputOboeCallback.cpp
        OpenGLContext.cpp
        PlatformAndroid.cpp
        SaveManager.cpp
        CartLoader.cpp
        EmulatorArgsBuilder.cpp
        SDCardArgsBuilder.cpp
        MelonInstance.cpp
        DSiSupport.cpp
        renderer/ScreenshotRenderer.cpp
        renderer/FrameQueue.cpp
        retroachievements/RetroAchievementsManager.cpp

        ../frontend/RewindManager.cpp
        ../frontend/mic_blow.h
)

set(OBOE ./oboe)
set(RCHEEVOS ./rcheevos)
set(LIBSLIRP ./libslirp)
add_subdirectory(${OBOE} ./oboe)
add_subdirectory(${RCHEEVOS} ./rcheevos)
add_subdirectory(../net ./net-utils)
add_library(melonDS-lib SHARED ${ANDROID_SOURCES})
find_library(android-lib android)
target_include_directories(${PROJECT_NAME} PUBLIC ${OBOE}/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${RCHEEVOS}/include)
target_include_directories(${PROJECT_NAME} PUBLIC ./)
target_include_directories(${PROJECT_NAME} PUBLIC ../)
target_include_directories(${PROJECT_NAME} PUBLIC ../frontend)
target_link_libraries(melonDS-lib ${android-lib} core net-utils oboe rcheevos EGL GLESv3)