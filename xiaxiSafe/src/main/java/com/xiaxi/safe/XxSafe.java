package com.xiaxi.safe;

import android.app.Activity;
import android.app.ActivityManager;
import android.app.Application;
import android.app.Instrumentation;
import android.app.Service;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.PowerManager;
import android.os.SystemClock;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Toast;


import java.net.NetworkInterface;
import java.util.Collections;
import java.util.Enumeration;
import java.util.List;

/**
 * 主安全類
 *
 * @author mrack
 */
public class XxSafe {
    static {
        System.loadLibrary(String.valueOf(new char[]{
                'x', 'x', '-', 'g', 'u', 'a', 'r', 'd'
        }));
    }

//    private static boolean isVpnUsed() {
//        try {
//            Enumeration<NetworkInterface> networkInterfaces = NetworkInterface.getNetworkInterfaces();
//            if (networkInterfaces != null) {
//                for (NetworkInterface networkInterface : Collections.list(networkInterfaces)) {
//                    if (networkInterface.isUp() && networkInterface.getInterfaceAddresses().size() != 0) {
//                        if ("tun0".equals(networkInterface.getName()) || "ppp0".equals(networkInterface.getName())) {
//                            return true;
//                        }
//                    }
//                }
//            }
//        } catch (Throwable th) {
//            th.printStackTrace();
//        }
//        return false;
//    }
//
//    private static String getAppName(Context context) {
//        PackageManager pm = context.getPackageManager();
//        //获取包信息
//        try {
//            PackageInfo packageInfo = pm.getPackageInfo(context.getPackageName(), 0);
//            //获取应用信息
//            ApplicationInfo applicationInfo = packageInfo.applicationInfo;
//            //获取albelRes
//            int labelRes = applicationInfo.labelRes;
//            //返回App的名称
//            return context.getResources().getString(labelRes);
//        } catch (PackageManager.NameNotFoundException e) {
//            e.printStackTrace();
//        }
//
//        return null;
//    }
//
//    private static Handler b = new Handler(Looper.getMainLooper());
//    private static Runnable c = new Runnable() {
//        @Override
//        public void run() {
//            List runningTasks = ((ActivityManager) application.getSystemService(Service.ACTIVITY_SERVICE)).getRunningTasks(1000);
//            String packageName = ((ActivityManager.RunningTaskInfo) runningTasks.get(0)).topActivity.getPackageName();
//            String packageName1 = application.getPackageName();
//            Log.d("XxSafe", "run: " + packageName + "\n" + packageName1);
//            if (!packageName.contains(application.getPackageName())) {
//                Toast.makeText(application, "“" + getAppName(application) + "”已进入到后台运行，请谨慎操作！", Toast.LENGTH_SHORT).show();
//            }
//        }
//    };
//    private static int d = 200;
//    private static Runnable e = () -> new Thread(() -> {
//        try {
//            Instrumentation instrumentation = new Instrumentation();
//            KeyEvent keyEvent = new KeyEvent(SystemClock.uptimeMillis(), SystemClock.uptimeMillis(), 1, 0, 0, 0, 16539, 0);
//            instrumentation.sendTrackballEventSync(MotionEvent.obtain(SystemClock.uptimeMillis(), SystemClock.uptimeMillis(), 1, 50.0f, 50.0f, 0));
//        } catch (Exception unused) {
//            b.removeCallbacks(c);
//            b.postDelayed(c, 200);
//        }
//    }).start();
//    private static Application application = null;
//    private static Application.ActivityLifecycleCallbacks lifecycleCallbacks = null;
//
//    static class WindowFocusView extends View {
//        public WindowFocusView(Context context) {
//            super(context);
//        }
//
//        @Override
//        public void onWindowFocusChanged(boolean z) {
//            super.onWindowFocusChanged(z);
//            onWindowFocusChanged1(z);
//
//        }
//    }
//
//    private static void checkActivitySafe(Application application, boolean screen, boolean hijack) {
//        if (Build.VERSION.SDK_INT >= 14) {
//            XxSafe.application = application;
//            if (!(lifecycleCallbacks == null || XxSafe.application == null)) {
//                //如果已经注册过了 Activity 的生命周期回调，则先注销
//                XxSafe.application.unregisterActivityLifecycleCallbacks(lifecycleCallbacks);
//            }
//            lifecycleCallbacks = new Application.ActivityLifecycleCallbacks() {
//                @Override
//                public void onActivitySaveInstanceState(Activity activity, Bundle bundle) {
//                }
//
//                @Override
//                public void onActivityCreated(final Activity activity, Bundle bundle) {
//                    if (screen) {
//                        //防止 Activity 被截屏和录屏
//                        activity.getWindow().addFlags(0x2000);
//                    }
//                    if (hijack) {
//                        b.removeCallbacks(c);
//                    }
//                }
//
//                @Override
//                public void onActivityStarted(Activity activity) {
//                    b.removeCallbacks(c);
//                }
//
//                @Override
//                public void onActivityResumed(Activity activity) {
//                    if (hijack) {
//                        b.removeCallbacks(c);
//                    }
//                }
//
//                @Override
//                public void onActivityPaused(Activity activity) {
//                    // Activity 暂停的时候
//                    if (hijack) {
//                        boolean isInteractive;
//                        b.removeCallbacks(c);
//                        PowerManager powerManager = (PowerManager) XxSafe.application.getSystemService(Service.POWER_SERVICE);
//                        //判断是否处于一个交互的状态
//                        if (Build.VERSION.SDK_INT >= 20) {
//                            isInteractive = powerManager.isInteractive();
//                        } else {
//                            isInteractive = powerManager.isScreenOn();
//                        }
//                        if (!isInteractive) {
//                            return;
//                        }
//                        k();
//                        onWindowFocusChanged1(false);
//                    }
//                }
//
//                @Override
//                public void onActivityStopped(Activity activity) {
//                    if (hijack) {
//                        boolean isInteractive;
//                        PowerManager powerManager = (PowerManager) XxSafe.application.getSystemService(Service.POWER_SERVICE);
//                        if (Build.VERSION.SDK_INT >= 20) {
//                            isInteractive = powerManager.isInteractive();
//                        } else {
//                            isInteractive = powerManager.isScreenOn();
//                        }
//                        if (isInteractive) {
//                            k();
//                        }
//                    }
//                }
//
//                @Override
//                public void onActivityDestroyed(Activity activity) {
//                    if (hijack) {
//                        b.removeCallbacks(c);
//                    }
//                }
//            };
//            XxSafe.application.registerActivityLifecycleCallbacks(lifecycleCallbacks);
//        }
//    }
//
//
//    private static void k() {
//        b.removeCallbacks(c);
//        b.removeCallbacks(e);
//        b.postDelayed(c, (long) d);
//    }
//
//    public static void onWindowFocusChanged1(boolean z) {
//        if (!z) {
//            k();
//        }
//    }


    /**
     * 开启保护(检测即退出)
     *
     * @param application Application
     */
    public static void protect(Application application) throws NoSuchFieldException, IllegalAccessException {
        protect(application, null);
    }

    /**
     * 开启保护(回调信息)
     *
     * @param application Application
     * @param process     检测回调
     */
    public static void protect(Application application, XxEventProcess process) throws NoSuchFieldException, IllegalAccessException {
        protect(application, process, application.getAssets());
    }



    private static native void protect(Application application, XxEventProcess process, AssetManager manager);

}
