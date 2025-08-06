package com.xiaxi.safe.util;

import android.content.Context;
import android.os.Build;
import android.provider.Settings;

import java.lang.reflect.Field;

public class EnvDetector {

    public static boolean isBootLoaderEnabled(Context context) throws Exception {
        AppConfig appConfig = new AppConfig(context);
        appConfig.doAttestation();
        return false;
    }
    public static boolean isDeveloperModeEnabled(Context context) {
        return Settings.Secure.getInt(
                context.getContentResolver(),
                Settings.Global.DEVELOPMENT_SETTINGS_ENABLED, 0) != 0;
    }

    public static boolean isAdbEnabled(Context context) {
        return Settings.Secure.getInt(
                context.getContentResolver(),
                Settings.Global.ADB_ENABLED, 0) != 0;
    }

    public static boolean isAdbWifiEnabled(Context context){
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            try {
                Field adbWifiEnabledField = Settings.Global.class.getDeclaredField("ADB_WIFI_ENABLED");
                adbWifiEnabledField.setAccessible(true);
                String adbWifiEnabledFieldValue = (String) adbWifiEnabledField.get(null);
                int adbWifiEnabled = Settings.Global.getInt(context.getContentResolver(), adbWifiEnabledFieldValue, 0);
                return adbWifiEnabled == 1;
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return false;
    };
}
