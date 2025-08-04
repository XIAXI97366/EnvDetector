package com.xiaxi.safe.util;

import android.content.pm.PackageManager;
import android.content.Context;
import android.os.Build;
import android.content.SharedPreferences;
import android.util.Log;

import java.io.IOException;
import java.security.KeyPairGenerator;
import java.security.KeyStore;
import java.security.KeyStoreException;
import android.security.keystore.KeyProperties;

import com.xx.shell.RiskCheckApplication;

public class AppConfig {
    private PackageManager pm = null;
    private Context appContext = null;
    private SharedPreferences sp = null;

    public boolean hasDeviceIds = false;
    public boolean hasStrongBox = false;
    public boolean hasAttestKey = false;
    public boolean hasSak = false;  // 当前设备是否支持 Samsung 的 SAK（Samsung Attestation Key，也可以获知当前是否为三星设备）功能

    private final KeyStore keyStore;

    private final KeyPairGenerator keyPairGenerator;

    public AppConfig(Context base) throws Exception {
        appContext = base;
        pm = base.getPackageManager();
        sp = base.getSharedPreferences("app_config", Context.MODE_PRIVATE);

        keyStore = KeyStore.getInstance("AndroidKeyStore");
        keyStore.load(null);
        keyPairGenerator = KeyPairGenerator.getInstance(
                KeyProperties.KEY_ALGORITHM_EC, "AndroidKeyStore");
    }

    public boolean hasStrongBox(){
        if (null != pm){
            // StrongBox Keymaster 是 Android 提供的一个 基于硬件的密钥保护机制，它运行在一个受信任环境中（如 TEE 或独立的安全元件 Secure Element）
            // | 功能             | 描述                                 |
            // | -------------- | ---------------------------------- |
            // | 硬件隔离           | 密钥永不离开硬件模块                         |
            // | 抗篡改            | 即使系统被 root，攻击者也难以读取 StrongBox 中的密钥 |
            // | 支持 attestation | 可用于生成强 attestation（证明密钥是在可信硬件中生成）  |
            // | 限制用途           | 可设置密钥用途、时间、认证条件等                   |
            hasStrongBox = Build.VERSION.SDK_INT >= Build.VERSION_CODES.P &&
                    pm.hasSystemFeature(PackageManager.FEATURE_STRONGBOX_KEYSTORE);
            return hasStrongBox;
        }
        return false;
    }
    private boolean hasDevicesIdAttestation(){
        if (null != pm){
            // hasDeviceIds == true 表示当前设备和系统支持 Device ID Attestation（设备标识证明），
            // 可以通过生成带有 attestation challenge 的密钥对，并从返回的证书链中读取设备信息，如品牌、型号、制造商等
            hasDeviceIds = Build.VERSION.SDK_INT >= Build.VERSION_CODES.P &&
                    pm.hasSystemFeature("android.software.device_id_attestation");
            return hasDeviceIds;
        }
        return false;
    }

    private boolean hasAttestationKey(){
        if (null != pm){
            // FEATURE_KEYSTORE_APP_ATTEST_KEY 是 Android 12（API 31）引入的一个系统特性，用于标识设备是否 支持 App Attestation Key（应用认证密钥）功能
            // App Attestation Key 是 Android 平台中引入的一种机制，允许特定设备为某个 App 创建一个专属的密钥对，并通过该密钥提供证明（Attestation），确保:
                // 密钥属于该 App（通常绑定 packageName + 签名）
                // 密钥在硬件中生成，并受可信执行环境（TEE）或 StrongBox 保护
                // 证明未被篡改、未导出、未伪造
                // 可附加设备属性或唯一 ID 等进行高级设备绑定
                // 简单说：它是一种安全地证明“我是某个设备上、某个 App 的原始副本”的方式，由 Android 系统签发 attestation 证书链，可供服务端验证
            hasAttestKey = Build.VERSION.SDK_INT >= Build.VERSION_CODES.S &&
                    pm.hasSystemFeature(PackageManager.FEATURE_KEYSTORE_APP_ATTEST_KEY);
            return hasAttestKey;
        }
        return false;
    }

    public void doAttestation(){
        String alias = null;
        String attestKeyAlias = null;
        boolean includeProps = hasDevicesIdAttestation();
        boolean useAttestKey = hasAttestationKey();
        boolean useStrongBox = hasStrongBox();

        alias = useStrongBox ? RiskCheckApplication.TAG + "_strongbox" : RiskCheckApplication.TAG;
        attestKeyAlias = useAttestKey ? alias + "_persistent" : null;

        if (useAttestKey && containsAlias(attestKeyAlias)){
            generateKeyPair(attestKeyAlias, attestKeyAlias, useStrongBox,
                    includeProps, uniqueIdIncluded, idFlags, false);
        }

        generateKeyPair(alias, attestKeyAlias, useStrongBox,
                includeProps, uniqueIdIncluded, idFlags, useSak);

    }


    public boolean containsAlias(String alias) {
        try {
            return keyStore.containsAlias(alias);
        } catch (KeyStoreException e) {
            Log.e(RiskCheckApplication.TAG, "containsAlias", e);
            throw new IllegalStateException(e.getMessage());
        }
    }
}
