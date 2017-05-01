#include <map>
#include <jni.h>
#include <android/log.h>
#include <android/native_window_jni.h>
#include "CppSurfaceView.h"

std::map<int, CppSurfaceView*> gpSufacesLists;

#ifdef __cplusplus
extern "C" {
#endif

void Java_com_test_a3rdpartylibpng_NativeFunc_create(JNIEnv *pEnv, jclass type, jint id) {
    gpSufacesLists[id] = new CppSurfaceView(id);
}

void Java_com_test_a3rdpartylibpng_NativeFunc_surfaceCreated(JNIEnv *pEnv, jclass type, jint id, jobject surface) {
    gpSufacesLists[id]->createThread(pEnv, surface);
}

void Java_com_test_a3rdpartylibpng_NativeFunc_surfaceChanged(JNIEnv *pEnv, jclass type, jint id, jint width, jint height) {
    gpSufacesLists[id]->isSurfaceCreated = true;
    gpSufacesLists[id]->DspW = width;
    gpSufacesLists[id]->DspH = height;
}

void Java_com_test_a3rdpartylibpng_NativeFunc_surfaceDestroyed(JNIEnv *pEnv, jclass type, jint id) {
    gpSufacesLists[id]->mStatus = CppSurfaceView::STATUS_FINISH;
    gpSufacesLists[id]->destroy();
}

#ifdef __cplusplus
}
#endif

/******************/
/* CppSurfaceView */
/******************/
const float CppSurfaceView::vertices[] = {
    -1.0f,  1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,
     1.0f, -1.0f, 0.0f
};

const float CppSurfaceView::texcoords[] = {
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 1.0f
};

CppSurfaceView::CppSurfaceView(int id) : mId(id), mStatus(CppSurfaceView::STATUS_NONE), mThreadId(-1) {
}

void CppSurfaceView::createThread(JNIEnv *pEnv, jobject surface) {
    mStatus = CppSurfaceView::STATUS_INITIALIZING;
    mWindow = ANativeWindow_fromSurface(pEnv, surface);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&mThreadId, &attr, CppSurfaceView::draw_thread, this);
}

void *CppSurfaceView::draw_thread(void *pArg) {
    if(pArg == NULL) return NULL;
    CppSurfaceView *pSurface = (CppSurfaceView*)pArg;

    pSurface->initEGL();
    pSurface->initGL();
    pSurface->mStatus = CppSurfaceView::STATUS_DRAWING;

    pSurface->predrawGL();
    struct timespec req = {0, (int)(16.66*1000000)};
    for(;pSurface->mStatus==CppSurfaceView::STATUS_DRAWING;) {

        /* SurfaceCreated()が動作した時は、画面サイズ変更を実行 */
        if(pSurface->isSurfaceCreated) {
            pSurface->isSurfaceCreated = false;
            glViewport(0,0,pSurface->DspW,pSurface->DspH);
        }

        /* 通常の描画処理 */
        pSurface->drawGL();
        nanosleep(&req, NULL);
    }

    pSurface->finEGL();

    return NULL;
}

void CppSurfaceView::initEGL() {
    EGLint major, minor;
    mEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(mEGLDisplay, &major, &minor);

    /* 設定取得 */
    const EGLint configAttributes[] = {
            EGL_LEVEL, 0,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            /* 透過設定 */
            EGL_ALPHA_SIZE, EGL_OPENGL_BIT,
            /*EGL_BUFFER_SIZE, 32 */  /* ARGB8888用 */
            EGL_DEPTH_SIZE, 16,
            EGL_NONE
    };

    EGLConfig config;
    EGLint numConfigs;
    eglChooseConfig(mEGLDisplay, configAttributes, &config, 1, &numConfigs);

    /* context生成 */
    const EGLint contextAttributes[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };
    mEGLContext = eglCreateContext(mEGLDisplay, config, EGL_NO_CONTEXT, contextAttributes);

    /* ウィンドウバッファサイズとフォーマットを設定 */
    EGLint format;
    eglGetConfigAttrib(mEGLDisplay, config, EGL_NATIVE_VISUAL_ID, &format);
    ANativeWindow_setBuffersGeometry(mWindow, 0, 0, format);

    /* surface生成 */
    mEGLSurface = eglCreateWindowSurface(mEGLDisplay, config, mWindow, NULL);
    if(mEGLSurface == EGL_NO_SURFACE) {
        __android_log_print(ANDROID_LOG_ERROR, "CppSurfaceView", "%d surface生成 失敗!!");
        return;
    }

    /* context再生成 */
    mEGLContext = eglCreateContext(mEGLDisplay, config, EGL_NO_CONTEXT, contextAttributes);

    /* 作成したsurface/contextを関連付け */
    if(eglMakeCurrent(mEGLDisplay, mEGLSurface, mEGLSurface, mEGLContext) == EGL_FALSE) {
        __android_log_print(ANDROID_LOG_ERROR, "CppSurfaceView", "%d surface/contextの関連付け 失敗!!");
        return;
    }

    /* チェック */
    EGLint w,h;
    eglQuerySurface(mEGLDisplay, mEGLSurface, EGL_WIDTH, &w);
    eglQuerySurface(mEGLDisplay, mEGLSurface, EGL_HEIGHT,&h);
    glViewport(0,0,w,h);
}

void CppSurfaceView::initGL() {
    program = createProgram(VERTEXSHADER, FRAGMENTSHADER);
}

GLuint CppSurfaceView::createProgram(const char *vertexshader, const char *fragmentshader) {
    GLuint vhandle = loadShader(GL_VERTEX_SHADER, vertexshader);
    if(vhandle == GL_FALSE) return GL_FALSE;

    GLuint fhandle = loadShader(GL_FRAGMENT_SHADER, fragmentshader);
    if(fhandle == GL_FALSE) return GL_FALSE;

    GLuint programhandle = glCreateProgram();
    if(programhandle == GL_FALSE) {
        checkGlError("glCreateProgram");
        return GL_FALSE;
    }

    glAttachShader(programhandle, vhandle);
    checkGlError("glAttachShader(VERTEX_SHADER)");
    glAttachShader(programhandle, fhandle);
    checkGlError("glAttachShader(FRAGMENT_SHADER)");

    glLinkProgram(programhandle);
    GLint linkStatus = GL_FALSE;
    glGetProgramiv(programhandle, GL_LINK_STATUS, &linkStatus);
    if(linkStatus != GL_TRUE) {
        GLint bufLen = 0;
        glGetProgramiv(programhandle, GL_INFO_LOG_LENGTH, &bufLen);
        if(bufLen) {
            char *logstr = (char*)malloc(bufLen);
            glGetProgramInfoLog(program, bufLen, NULL, logstr);
            __android_log_print(ANDROID_LOG_ERROR, "CppSurfaceView", "%d glLinkProgram() Fail!!\n %s", mId, logstr);
            free(logstr);
        }
        glDeleteProgram(programhandle);
        programhandle = GL_FALSE;
    }

    return programhandle;
}

GLuint CppSurfaceView::loadShader(int shadertype, const char *sourcestring) {
    GLuint shaderhandle = glCreateShader(shadertype);
    if(!shaderhandle) return GL_FALSE;

    glShaderSource(shaderhandle, 1, &sourcestring, NULL);
    glCompileShader(shaderhandle);

    GLint compiled = GL_FALSE;
    glGetShaderiv(shaderhandle, GL_COMPILE_STATUS, &compiled);
    if(!compiled) {
        GLint infoLen = 0;
        glGetShaderiv(shaderhandle, GL_INFO_LOG_LENGTH, &infoLen);
        if(infoLen) {
            char *logbuf = (char*)malloc(infoLen);
            if(logbuf) {
                glGetShaderInfoLog(shaderhandle, infoLen, NULL, logbuf);
                __android_log_print(ANDROID_LOG_ERROR, "CppSurfaceView", "%d shader failuer!!\n%s", mId, logbuf);
                free(logbuf);
            }
        }
        glDeleteShader(shaderhandle);
        shaderhandle = GL_FALSE;
    }

    return shaderhandle;
}

void CppSurfaceView::checkGlError(const char *argstr) {
    for(GLuint error = glGetError(); error; error = glGetError())
        __android_log_print(ANDROID_LOG_ERROR, "CppSurfaceView", "%d after %s errcode=%d", mId, argstr, error);
}

#include "CppPng.h"
void CppSurfaceView::predrawGL() {
    position = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(position);
    texcoord = glGetAttribLocation(program, "texcoord");
    glEnableVertexAttribArray(texcoord);

    glUseProgram(program);

    glGenTextures(1, &textures);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures);

    int sampler = glGetUniformLocation(program, "texture");

    CppPng png("/data/local/sky_cloud.png");
    __android_log_print(ANDROID_LOG_DEBUG, "aaaaa",
        "aaaaa Image: alpha:%d width:%dpx height%dpx", png.has_alpha(), png.get_width(), png.get_height());

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, png.has_alpha() ? GL_RGBA : GL_RGB, png.get_width(), png.get_height(),
                 0, png.has_alpha() ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, png.get_data());

    glClearColor(0, 0, 0, 0);
}

void CppSurfaceView::drawGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glClear(GL_COLOR_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, textures);
    glVertexAttribPointer(texcoord, 2, GL_FLOAT, GL_FALSE, 0, texcoords);
    glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    eglSwapBuffers(mEGLDisplay, mEGLSurface);
}

void CppSurfaceView::finEGL() {
    __android_log_print(ANDROID_LOG_DEBUG, "aaaaa", "aaaaa%d CppSurfaceView::finEGL()", mId);
    ANativeWindow_release(mWindow);
    mWindow = NULL;
}

void CppSurfaceView::destroy() {
    __android_log_print(ANDROID_LOG_DEBUG, "aaaaa", "aaaaa%d CppSurfaceView::destroy()", mId);
    pthread_join(mThreadId, NULL);
}

CppSurfaceView::~CppSurfaceView() {
}
