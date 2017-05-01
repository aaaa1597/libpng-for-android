#ifndef TESTNATIVESURFACE_H
#define TESTNATIVESURFACE_H

#include <jni.h>
#include <android/native_window.h>
#include <pthread.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_com_test_a3rdpartylibpng_NativeFunc_create(JNIEnv *env, jclass type, jint id);
JNIEXPORT void JNICALL Java_com_test_a3rdpartylibpng_NativeFunc_surfaceCreated(JNIEnv *env, jclass type, jint id, jobject surface);
JNIEXPORT void JNICALL Java_com_test_a3rdpartylibpng_NativeFunc_surfaceChanged(JNIEnv *env, jclass type, jint id, jint width, jint height);
JNIEXPORT void JNICALL Java_com_test_a3rdpartylibpng_NativeFunc_surfaceDestroyed(JNIEnv *env, jclass type, jint id);

#ifdef __cplusplus
}
#endif

class CppSurfaceView {
private:
    const char *VERTEXSHADER =
        "attribute vec4 position;\n"
        "attribute vec2 texcoord;\n"
        "varying vec2 texcoordVarying;\n"
        "void main() {\n"
        "    gl_Position = position;\n"
        "    texcoordVarying = texcoord;\n"
        "}\n";

    const char *FRAGMENTSHADER =
        "precision mediump float;\n"
        "varying vec2 texcoordVarying;\n"
        "uniform sampler2D texture;\n"
        "void main() {\n"
        "    gl_FragColor = texture2D(texture, texcoordVarying);\n"
        "}\n";

    static const float vertices[];
    static const float texcoords[];

private:
    GLuint program = -1;
    GLuint position = -1;
    GLuint texcoord = -1;
    GLuint textures = -1;

private:
    GLuint createProgram(const char *vertexshader, const char *fragmentshader);
    GLuint loadShader(int i, const char *vertexshader);
    void checkGlError(const char *argstr);

public:
    static const int STATUS_NONE   = 0;
    static const int STATUS_INITIALIZING = 1;
    static const int STATUS_DRAWING= 2;
    static const int STATUS_FINISH = 3;
    int mStatus = STATUS_NONE;
    int mId = -1;
    pthread_t mThreadId = -1;
    ANativeWindow *mWindow = NULL;
    EGLDisplay mEGLDisplay = NULL;
    EGLContext mEGLContext = NULL;
    EGLSurface mEGLSurface = NULL;
    bool isSurfaceCreated = false;
    int DspW = 0;
    int DspH = 0;

public:
    CppSurfaceView(int id);
    virtual ~CppSurfaceView();
    static void *draw_thread(void *pArg);
    void createThread(JNIEnv *pEnv, jobject surface);
    void initEGL();
    void finEGL();
    void initGL();
    void predrawGL();
    void drawGL();
    void destroy();
};

#endif //TESTNATIVESURFACE_H
