package com.xiaxi.safe.util;

import android.annotation.SuppressLint;
import android.app.admin.DevicePolicyManager;
import android.content.pm.PackageManager;
import android.content.Context;
import android.os.Build;
import android.content.SharedPreferences;
import android.os.RemoteException;
import android.security.keystore.KeyGenParameterSpec;
import android.security.keystore.KeyGenParameterSpec_rename;
import android.security.keystore.StrongBoxUnavailableException;
import android.util.Log;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.security.KeyPairGenerator;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.ProviderException;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.security.spec.ECGenParameterSpec;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.List;
import java.util.Objects;

import android.security.keystore.KeyProperties;

import androidx.annotation.RequiresApi;

import com.xx.shell.RiskCheckApplication;

import javax.security.auth.x500.X500Principal;

/**
 * 该类作用：判断设备是否支持 AttestKey 、 DevicesIdAttestation 特性并对应生成证书并保存
 */
public class KeyAttestation {
    private PackageManager pm = null;
    public boolean hasDeviceIds = false;
    public boolean hasStrongBox = false;
    public boolean hasAttestKey = false;
    public boolean hasSak = false;  // 当前设备是否支持 Samsung 的 SAK（Samsung Attestation Key，也可以获知当前是否为三星设备）功能
    private final KeyStore keyStore;
    private final KeyPairGenerator keyPairGenerator;
    private final List<X509Certificate> currentCerts;
    private final CertificateFactory factory;

    public KeyAttestation(Context base) throws Exception {
        pm = base.getPackageManager();
        keyStore = KeyStore.getInstance("AndroidKeyStore");
        keyStore.load(null);
        keyPairGenerator = KeyPairGenerator.getInstance(
                KeyProperties.KEY_ALGORITHM_EC, "AndroidKeyStore");
        currentCerts = new ArrayList<>();
        factory = CertificateFactory.getInstance("X.509");
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

        // 判断是否支持 DevicesIdAttestation、AttestationKey 特性
        // 目前除去 AttestationKey 和 DevicesIdAttestation 其余特性暂不支持
        // 所以也就暂不支持三星设备、StrongBox、uniqueIdIncluded
        includeProps = hasDevicesIdAttestation();
        useAttestKey = hasAttestationKey();
        alias = useStrongBox ? RiskCheckApplication.TAG + "_strongbox" : RiskCheckApplication.TAG;
        attestKeyAlias = useAttestKey ? alias + "_persistent" : null;
        currentCerts.clear();

        try{
            // 先用 containsAlias 判断当前 KeyStore 中是否存在名为 alias 的条目（证书/密钥/密钥对）
            // 返回 true 表示该别名在此 KeyStore 中的 keyentry 表中已存在，否则 false
            if (useAttestKey && !containsAlias(attestKeyAlias)){
                generateKeyPair(attestKeyAlias, attestKeyAlias, useStrongBox,
                        includeProps, uniqueIdIncluded, idFlags, false);
                // 上述代码调用 generateKeyPair 的为创建持久化证明密钥（Persistent Attestation Key）
                // 若 alias 参数 == attestKeyAlias 参数，通常表示在生成“持久化证明密钥（PAK）
                // 那么 genParameter 方法会把用途设为 PURPOSE_ATTEST_KEY，并设置证书主题为 CN=App Attest Key（仅用于签发证明，不做业务签名）
                // 只在本应用 Keystore 命名空间下创建一次，后续可复用；存在就跳过，避免重复生成
            }
            generateKeyPair(alias, attestKeyAlias, useStrongBox,
                    includeProps, uniqueIdIncluded, idFlags, useSak);
            // 再次调用 generateKeyPair 目的为生成主业务密钥（通常用途为 PURPOSE_SIGN 等），并在参数中指定 attestKeyAlias
            // 系统据此用上一步的生成的 Persistent Attestation Key 为这把主密钥出具 attestation 证书链
            // 备注：三星机型时 useSak 为 true 会走 SAK 流程（厂商扩展）以获取/存储证书链
            // 备注：生成后通常会 getCertChain(alias)，若需要也会取 getCertChain(attestKeyAlias)

            // 先获取业务密钥 alias 的证书链
            getCertChain(alias);
            if (useAttestKey) {
                // 如果启用了持久化证明密钥（PAK），也取它的证书链
                getCertChain(attestKeyAlias);
            }

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

    // containsAlias 判断当前 KeyStore 中是否存在名为 alias 的条目（证书/密钥/密钥对）
    // 返回 true 表示该别名在此 KeyStore 中的 keyentry 表中已存在，否则 false
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
            // 调用 generateKeyPair() 真实生成密钥对
            keyPairGenerator.generateKeyPair();
            if (useSak) {
                // 此处是针对三星设备的处理
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
        // attestKey == true → 生成的是“证明用密钥”（PAK），用途设为 KeyProperties.PURPOSE_ATTEST_KEY，表示这把密钥仅用于出具 attestation 证明，而不用于业务签名/加解密（Android 12+ 支持）
        // attestKey == false → 生成的是“业务密钥”，用途设为 KeyProperties.PURPOSE_SIGN，表示可执行签名等业务操作，attestation 由上面那把 PAK 为其出具

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
                // 如果 alias == attestKeyAlias 那么本次目的就是生成持久化证明密钥（PAK）
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

    private void getCertChain(String alias) throws RemoteException, CertificateException {
        // 拿到连续拼接的 DER 编码证书字节
        var certChain = getCertificateChain(alias);
        if (certChain == null) {
            throw new ProviderException("Unable to get certificate chain");
        }
        // 用证书工厂解析为证书对象列表并缓存
        generateCertificates(new ByteArrayInputStream(certChain));
    }

    /**
     * 从 Android Keystore 获取指定 alias 的证书链，并将每张证书的 DER 编码顺序写入到一个 byte[] 返回。
     * 返回的是“拼接后的二进制”，便于一次性用 CertificateFactory 解析。
     */
    public byte[] getCertificateChain(String alias) {
        try {
            var chain = keyStore.getCertificateChain(alias);
            if (chain == null) {
                return null;
            }
            var buf = new ByteArrayOutputStream(8192);
            for (var cert : chain) {
                // 每张证书按 DER 编码追加写入
                buf.write(cert.getEncoded());
            }
            return buf.toByteArray();
        } catch (Exception e) {
            Log.e(RiskCheckApplication.TAG, "getCertificateChain", e);
            throw new IllegalStateException(e.getMessage());
        }
    }

    /**
     * 使用 CertificateFactory 从输入流解析出一组 X509 证书，并追加到 currentCerts。
     * 若解析为空说明输入不合法，抛出 CertificateException。
     */
    private void generateCertificates(InputStream in) throws CertificateException {
        var list = (List<X509Certificate>) factory.generateCertificates(in);
        if (list.isEmpty()) {
            throw new CertificateException("No certificate");
        }
        // 存放（解析的 alias 与 attestKeyAlias 的证书链）
        currentCerts.addAll(list);
    }
}
