package com.xiaxi.safe.util;

import android.content.Context;
import android.os.Build;
import android.provider.Settings;

import com.xiaxi.safe.util.KeyAttestation.KeyAttestation;
import java.lang.reflect.Field;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Collections;
import java.util.Enumeration;

public class EnvDetector {
    private static KeyAttestation keyAttestation = null;

    public static KeyAttestation getKeyAttestation(){
        if (null == keyAttestation){
            return null;
        }else{
            return keyAttestation;
        }
    }

    /** 在 native 中的 xx_protect 函数中反射调用 **/
    public static boolean isDeveloperModeEnable(Context context) {
        return Settings.Secure.getInt(
                context.getContentResolver(),
                Settings.Global.DEVELOPMENT_SETTINGS_ENABLED, 0) != 0;
    }

    /** 在 native 中的 xx_protect 函数中反射调用 **/
    public static boolean isAdbEnable(Context context) {
        return Settings.Secure.getInt(
                context.getContentResolver(),
                Settings.Global.ADB_ENABLED, 0) != 0;
    }

    /** 在 native 中的 xx_protect 函数中反射调用 **/
    public static boolean isWifiAdbEnable(Context context){
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
    }

    /**
     * 优先以该函数获取的设备信息为基准，但如果无法从 hardwareEnforced 中获取设备信息（品牌、设备、产品、制造商、机型）
     * 则调用 native 端的 romEnv::getDeviceInfo() 获取如下设备信息
     * initKeyAttestation 和 showDeviceInfo 是配套函数
     */
    public static void initKeyAttestation(Context context)throws Exception{
        keyAttestation = new KeyAttestation(context);
        keyAttestation.doAttestation();
    }

    public static String showDeviceInfo() {
        StringBuilder deviceInfo = new StringBuilder();

        if (null == keyAttestation || null == keyAttestation.attestationData){
            return "appConfig or appConfig.attestationData value is null";
        }

        if (keyAttestation.attestationData.showAttestation.getTeeEnforced().getBrand() != null) {
            deviceInfo.append("\n品牌 : ").append(keyAttestation.attestationData.showAttestation.
                    getTeeEnforced().getBrand());
        }else{
            deviceInfo.append("\n品牌 : ").append(getDeviceBrand());
        }

        if (keyAttestation.attestationData.showAttestation.getTeeEnforced().getDevice() != null) {
            deviceInfo.append("\n设备 : ").append(keyAttestation.attestationData.showAttestation.
                    getTeeEnforced().getDevice());
        }else{
            deviceInfo.append("\n设备 : ").append(getDeviceDevice());
        }

        if (keyAttestation.attestationData.showAttestation.getTeeEnforced().getManufacturer() != null) {
            deviceInfo.append("\n制造商 : ").append(keyAttestation.attestationData.showAttestation.
                    getTeeEnforced().getManufacturer());
        }else{
            deviceInfo.append("\n制造商 : ").append(getDeviceManufacturer());
        }

        if (keyAttestation.attestationData.showAttestation.getTeeEnforced().getModel() != null) {
            deviceInfo.append("\n机型 : ").append(keyAttestation.attestationData.showAttestation.
                    getTeeEnforced().getModel());
        }else{
            deviceInfo.append("\n机型 : ").append(getDeviceModel());
        }

        if (keyAttestation.attestationData.showAttestation.getTeeEnforced().getProduct() != null) {
            deviceInfo.append("\n产品 : ").append(keyAttestation.attestationData.showAttestation.
                    getTeeEnforced().getProduct());
        }else{
            deviceInfo.append("\n产品 : ").append(getDeviceProduct());
        }

        return deviceInfo.toString();
    }

    /** 在 native 中的 xx_protect 函数中反射调用 **/
    public static boolean isVpnUsed() {
        try {
            Enumeration<NetworkInterface> networkInterfaces = NetworkInterface.getNetworkInterfaces();
            if (null != networkInterfaces) {
                for (NetworkInterface networkInterface : Collections.list(networkInterfaces)) {
                    if (networkInterface.isUp() && networkInterface.getInterfaceAddresses().size() != 0) {
                        if ("tun0".equals(networkInterface.getName()) || "ppp0".equals(networkInterface.getName())) {
                            return true;
                        }
                    }
                }
            }
        }catch (SocketException e){
            e.printStackTrace();
        }
        return false;
    }

    private static native String getDeviceBrand();
    private static native String getDeviceDevice();
    private static native String getDeviceManufacturer();
    private static native String getDeviceModel();
    private static native String getDeviceProduct();
    public static native boolean isBootLoaderEnabled(KeyAttestation keyAttestation);
    public static native boolean checkSignatureV2();
}

