package com.test.a3rdpartylibpng;

import android.graphics.PixelFormat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        SurfaceView surface = ((SurfaceView)findViewById(R.id.surface));

        /* 透過設定 */
        surface.getHolder().setFormat(PixelFormat.TRANSLUCENT);
        surface.setZOrderOnTop(true);

        /* コールバック設定 */
        surface.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                /* C++ */
                NativeFunc.create(0);
                NativeFunc.surfaceCreated(0, holder.getSurface());
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                NativeFunc.surfaceChanged(0, width, height);
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                NativeFunc.surfaceDestroyed(0);
            }
        });
    }
}
