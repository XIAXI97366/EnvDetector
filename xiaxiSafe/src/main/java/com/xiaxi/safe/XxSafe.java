package com.xiaxi.safe;

import android.app.Application;
import android.content.res.AssetManager;

public class XxSafe {
    static {
        System.loadLibrary(String.valueOf(new char[]{
                'x', 'x', '-', 'g', 'u', 'a', 'r', 'd'
        }));
    }

    public static void protect(Application application) throws NoSuchFieldException, IllegalAccessException {
        protect(application, null);
    }

    public static void protect(Application application, XxEventProcess process)
            throws NoSuchFieldException, IllegalAccessException {
        protect(application, process, application.getAssets());
    }

    private static native void protect(Application application, XxEventProcess process, AssetManager manager);
}
