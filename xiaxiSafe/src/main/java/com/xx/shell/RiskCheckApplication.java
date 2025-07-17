package com.xx.shell;


import android.app.Application;
import android.content.Context;
import android.util.Log;

import com.xiaxi.safe.XxSafe;
import com.xiaxi.safe.util.EnvUtil;

//import com.xx.api.rposed.RC_MethodHook;
//import com.xx.api.rposed.RposedHelpers;


import me.weishu.reflection.Reflection;


public class RiskCheckApplication extends Application {
    private static final int REQUEST_CODE_PERMISSION = 100;

    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);
        Reflection.unseal(base);        //使用weishu的反射库
        Log.e("XIAXI", "DeveloperMode: " + EnvUtil.isDeveloperModeEnabled(base));
        Log.e("XIAXI", "Adb: " + EnvUtil.isAdbEnabled(base));
        Log.e("XIAXI", "Wifi Adb: " + EnvUtil.isAdbWifiEnabled(base));
        try {
            XxSafe.protect(this);
        } catch (NoSuchFieldException e) {
            throw new RuntimeException(e);
        } catch (IllegalAccessException e) {
            throw new RuntimeException(e);
        }


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
    }
}
