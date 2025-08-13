package com.xiaxi.safe.util;

import android.annotation.SuppressLint;
import android.app.admin.DevicePolicyManager;
import android.content.pm.PackageManager;
import android.content.Context;
import android.os.Build;
import android.content.SharedPreferences;
import android.security.keystore.KeyGenParameterSpec;
import android.security.keystore.KeyGenParameterSpec_rename;
import android.security.keystore.StrongBoxUnavailableException;
import android.util.Log;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.security.KeyPairGenerator;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.ProviderException;
import java.security.spec.ECGenParameterSpec;
import java.util.Arrays;
import java.util.Date;
import java.util.Objects;

import android.security.keystore.KeyProperties;

import androidx.annotation.RequiresApi;

import com.xx.shell.RiskCheckApplication;

import javax.security.auth.x500.X500Principal;

import com.xiaxi.safe.util.AttestationException;

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

    private boolean hasStrongBox(){
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

    public void doAttestation() throws AttestationException{
        String alias = null;
        String attestKeyAlias = null;
        boolean useStrongBox = false;
        boolean useAttestKey = false;
        boolean includeProps = false;
        boolean uniqueIdIncluded = false;
        int idFlags = 0;
        boolean useSak = false;

        includeProps = hasDevicesIdAttestation();
        useAttestKey = hasAttestationKey();
        alias = useStrongBox ? RiskCheckApplication.TAG + "_strongbox" : RiskCheckApplication.TAG;
        attestKeyAlias = useAttestKey ? alias + "_persistent" : null;
        try{
            // 除去 AttestationKey 和  DevicesIdAttestation 其余属性暂不支持
            if (useAttestKey && !containsAlias(attestKeyAlias)){
                generateKeyPair(attestKeyAlias, attestKeyAlias, useStrongBox,
                        includeProps, uniqueIdIncluded, idFlags, false);
            }
            generateKeyPair(alias, attestKeyAlias, useStrongBox,
                    includeProps, uniqueIdIncluded, idFlags, useSak);
        }catch (ProviderException e){
            Throwable cause = e.getCause();
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P
                    && e instanceof StrongBoxUnavailableException) {
                throw new AttestationException(AttestationException.CODE_STRONGBOX_UNAVAILABLE, e);
            } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU
                    && cause instanceof android.security.KeyStoreException keyStoreException) {
                throw toAttestationException(keyStoreException, e);
            } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O
                    && cause instanceof Exception) {
                throw new AttestationException(AttestationException.CODE_DEVICEIDS_UNAVAILABLE, e);
            } else if (cause != null && cause.toString().contains("device ids")) {
                throw new AttestationException(AttestationException.CODE_DEVICEIDS_UNAVAILABLE, e);
            } else {
                throw new AttestationException(AttestationException.CODE_UNAVAILABLE, e);
            }
        } catch (Exception e) {
            throw new AttestationException(AttestationException.CODE_UNKNOWN, e);
        }
    }


    public boolean containsAlias(String alias) {
        try {
            return keyStore.containsAlias(alias);
        } catch (KeyStoreException e) {
            Log.e(RiskCheckApplication.TAG, "containsAlias", e);
            throw new IllegalStateException(e.getMessage());
        }
    }

    private void generateKeyPair(String alias, String attestKeyAlias,
                                 boolean useStrongBox, boolean includeProps,
                                 boolean uniqueIdIncluded, int idFlags,
                                 boolean useSak) throws Exception {
        byte[] data = generateKeyPair2(alias, attestKeyAlias, useStrongBox,
                includeProps, uniqueIdIncluded, idFlags, useSak);
        if (data != null) {
            try (ObjectInputStream  it = new ObjectInputStream((new ByteArrayInputStream(data)))) {
                throw (Exception) it.readObject();
            }
        }
    }

    private byte[] generateKeyPair2(String alias,
                                  String attestKeyAlias,
                                  boolean useStrongBox,
                                  boolean includeProps,
                                  boolean uniqueIdIncluded,
                                  int idFlags,
                                  boolean useSak) {
        KeyGenParameterSpec params = (KeyGenParameterSpec) genParameter(alias, attestKeyAlias, useStrongBox,
                includeProps, uniqueIdIncluded, flagsToArray(idFlags));
        try {
            keyPairGenerator.initialize(params);
            keyPairGenerator.generateKeyPair();
            if (useSak) {
//                var utils = new com.samsung.android.security.keystore.AttestationUtils();
//                var spec = genSakParameter(params);
//                Iterable<byte[]> certChain;
//                if (spec.isDeviceAttestation()) {
//                    certChain = utils.attestDevice(spec);
//                } else {
//                    certChain = utils.attestKey(spec);
//                }
//                utils.storeCertificateChain(alias, certChain);
            }
            return null;
        } catch (Exception exception) {
            Log.e(RiskCheckApplication.TAG, "generateKeyPair", exception);
            ByteArrayOutputStream buf = new ByteArrayOutputStream(2048);
            try (ObjectOutputStream out = new ObjectOutputStream(buf)) {
                out.writeObject(exception);
            } catch (IOException e) {
                throw new IllegalStateException(e.getMessage());
            }
            return buf.toByteArray();
        }
    }

    private Object genParameter(String alias,
                                       String attestKeyAlias,
                                       boolean useStrongBox,
                                       boolean includeProps,
                                       boolean uniqueIdIncluded,
                                       int[] attestationIds) {
        Date now = new Date();
        boolean attestKey = Objects.equals(alias, attestKeyAlias);
        int purposes = attestKey ? KeyProperties.PURPOSE_ATTEST_KEY : KeyProperties.PURPOSE_SIGN;

        var builder = new KeyGenParameterSpec_rename.Builder(alias, purposes)
                .setAlgorithmParameterSpec(new ECGenParameterSpec("secp256r1"))
                .setDigests(KeyProperties.DIGEST_SHA256)
                .setCertificateNotBefore(now)
                .setAttestationChallenge(now.toString().getBytes());
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P && useStrongBox) {
            builder.setIsStrongBoxBacked(true);
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            if (includeProps) {
                builder.setDevicePropertiesAttestationIncluded(true);
            }
            if (attestationIds != null) {
                builder.setAttestationIds(attestationIds);
            }
            if (attestKey) {
                builder.setCertificateSubject(new X500Principal("CN=App Attest Key"));
            } else {
                builder.setAttestKeyAlias(attestKeyAlias);
            }
        }
        if (uniqueIdIncluded) {
            builder.setUniqueIdIncluded(true);
        }
        return builder.build();
    }

    private int[] flagsToArray(int idFlags) {
        int i = 0;
        int[] array = new int[3];
        if ((idFlags & DevicePolicyManager.ID_TYPE_SERIAL) != 0) {
            array[i++] = 1;
        }
        if ((idFlags & DevicePolicyManager.ID_TYPE_IMEI) != 0) {
            array[i++] = 2;
        }
        if ((idFlags & DevicePolicyManager.ID_TYPE_MEID) != 0) {
            array[i++] = 3;
        }
        return Arrays.copyOf(array, i);
    }


    @SuppressLint("SwitchIntDef")
    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    private AttestationException toAttestationException(android.security.KeyStoreException exception,
                                                               Exception e) {
        int code = exception.getNumericErrorCode();
        if (code == android.security.KeyStoreException.ERROR_ID_ATTESTATION_FAILURE) {
            return new AttestationException(AttestationException.CODE_DEVICEIDS_UNAVAILABLE, e);
        }
        if (code >= android.security.KeyStoreException.ERROR_ATTESTATION_KEYS_UNAVAILABLE) {
            if (exception.isTransientFailure()) {
                return new AttestationException(AttestationException.CODE_OUT_OF_KEYS_TRANSIENT, e);
            } else {
                return new AttestationException(AttestationException.CODE_OUT_OF_KEYS, e);
            }
        }
        if (code == android.security.KeyStoreException.ERROR_KEYMINT_FAILURE) {
            if (exception.toString().contains("ATTESTATION_KEYS_NOT_PROVISIONED")) {
                return new AttestationException(AttestationException.CODE_KEYS_NOT_PROVISIONED, e);
            }
        }
        if (exception.isTransientFailure()) {
            return new AttestationException(AttestationException.CODE_UNAVAILABLE_TRANSIENT, e);
        } else {
            return new AttestationException(AttestationException.CODE_UNAVAILABLE, e);
        }
    }
}
