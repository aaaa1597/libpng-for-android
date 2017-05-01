package com.test.a3rdpartylibpng;

import android.view.Surface;

/**
 * Created by jun on 2017/05/01.
 */
public class NativeFunc {
    static { System.loadLibrary("testlib"); }

    public native static void create(int id);
    public native static void surfaceCreated(int id, Surface surface);
    public native static void surfaceChanged(int id, int width, int height);
    public native static void surfaceDestroyed(int id);
}
