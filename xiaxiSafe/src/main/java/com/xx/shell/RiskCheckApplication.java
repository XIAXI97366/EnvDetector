package com.xx.shell;


import android.app.Application;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;
import android.util.Log;

import com.xiaxi.safe.XxSafe;
import com.xiaxi.safe.util.EnvDetector;

//import com.xx.api.rposed.RC_MethodHook;
//import com.xx.api.rposed.RposedHelpers;


import me.weishu.reflection.Reflection;


public class RiskCheckApplication extends Application {
    private static final int REQUEST_CODE_PERMISSION = 100;

    public static String TAG = "XIAXI";
    public Context appContext = null;

    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);
        appContext = base;
        Reflection.unseal(base);        //使用weishu的反射库

        Log.e(RiskCheckApplication.TAG, "DeveloperMode: " + EnvDetector.isDeveloperModeEnabled(base));
        Log.e(RiskCheckApplication.TAG, "Adb: " + EnvDetector.isAdbEnabled(base));
        Log.e(RiskCheckApplication.TAG, "Wifi Adb: " + EnvDetector.isAdbWifiEnabled(base));




//        ClassLoader classLoader = base.getClassLoader();
//        XposedHelpers.findAndHookMethod("com.example.checkrom.MainActivity", classLoader,
//                "stringFromJNI",
//                new XC_MethodHook() {
//                    @Override
//                    protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
//                        super.beforeHookedMethod(param);
//                    }
//
//                    @Override
//                    protected void afterHookedMethod(MethodHookParam param) throws Throwable {
//                        super.afterHookedMethod(param);
//                        Log.e("XIAXI", "param ->: " + (String) param.getResult());
//                        param.setResult(new String("lsplant hook is succesfull!!!!"));
//                    }
//                });
    }

    @Override
    public void onCreate() {
        super.onCreate();

        try {
            // 要放在 kApplication 的 onCreate 中，attachBaseContext 时间过早
                // Context 可能还没有完全初始化
                // 某些系统服务可能还不可用
                // PackageManager 等系统组件可能还未准备好
            Log.e(RiskCheckApplication.TAG, "BootLoader: " + EnvDetector.isBootLoaderEnabled(appContext));
            XxSafe.protect(this);
        } catch (NoSuchFieldException e) {
            throw new RuntimeException(e);
        } catch (IllegalAccessException e) {
            throw new RuntimeException(e);
        } catch (Exception e) {
            throw new RuntimeException(e);
        }

    }
}
