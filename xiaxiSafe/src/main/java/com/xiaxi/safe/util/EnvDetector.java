package com.xiaxi.safe.util;

import android.content.Context;
import android.os.Build;
import android.provider.Settings;

import com.xiaxi.safe.util.KeyAttestation.KeyAttestation;
import java.lang.reflect.Field;

public class EnvDetector {
    private static KeyAttestation keyAttestation = null;

    public static KeyAttestation getKeyAttestation(){
        if (null == keyAttestation){
            return null;
        }else{
            return keyAttestation;
        }
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

    /**
     * 优先以该函数获取的设备信息为基准，但如果无法从 hardwareEnforced 中获取设备信息（品牌、设备、产品、制造商、机型）
     * 则调用 native 端的 romEnv::getDeviceInfo() 获取如下设备信息
     * @return
     */
    public static String showDeviceInfo(Context context) throws Exception {
        keyAttestation = new KeyAttestation(context);
        StringBuilder deviceInfo = new StringBuilder();
        keyAttestation.doAttestation();

        if (null == keyAttestation.attestationData || null == keyAttestation){
            return "appConfig or appConfig.attestationData value is null";
        }

        if (keyAttestation.attestationData.showAttestation.getTeeEnforced().getBrand() != null) {
            deviceInfo.append("\n品牌 : ").append(keyAttestation.attestationData.showAttestation.getTeeEnforced().getBrand());
        }else{
            deviceInfo.append("\n品牌 : ").append(getDeviceBrand());
        }

        if (keyAttestation.attestationData.showAttestation.getTeeEnforced().getDevice() != null) {
            deviceInfo.append("\n设备 : ").append(keyAttestation.attestationData.showAttestation.getTeeEnforced().getDevice());
        }else{
            deviceInfo.append("\n设备 : ").append(getDeviceDevice());
        }

        if (keyAttestation.attestationData.showAttestation.getTeeEnforced().getManufacturer() != null) {
            deviceInfo.append("\n制造商 : ").append(keyAttestation.attestationData.showAttestation.getTeeEnforced().getManufacturer());
        }else{
            deviceInfo.append("\n制造商 : ").append(getDeviceManufacturer());
        }

        if (keyAttestation.attestationData.showAttestation.getTeeEnforced().getModel() != null) {
            deviceInfo.append("\n机型 : ").append(keyAttestation.attestationData.showAttestation.getTeeEnforced().getModel());
        }else{
            deviceInfo.append("\n机型 : ").append(getDeviceModel());
        }

        if (keyAttestation.attestationData.showAttestation.getTeeEnforced().getProduct() != null) {
            deviceInfo.append("\n产品 : ").append(keyAttestation.attestationData.showAttestation.getTeeEnforced().getProduct());
        }else{
            deviceInfo.append("\n产品 : ").append(getDeviceProduct());
        }

        return deviceInfo.toString();
    }

    private static native String getDeviceBrand();
    private static native String getDeviceDevice();
    private static native String getDeviceManufacturer();
    private static native String getDeviceModel();
    private static native String getDeviceProduct();
    public static native boolean isBootLoaderEnabled(KeyAttestation keyAttestation);
}

