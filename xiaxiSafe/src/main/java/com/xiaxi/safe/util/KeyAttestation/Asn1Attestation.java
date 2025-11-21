package com.xiaxi.safe.util.KeyAttestation;

import org.bouncycastle.asn1.ASN1Sequence;

import java.security.cert.CertificateParsingException;
import java.security.cert.X509Certificate;

public class Asn1Attestation extends Attestation {
    static final int ATTESTATION_VERSION_INDEX = 0;
    static final int ATTESTATION_SECURITY_LEVEL_INDEX = 1;
    static final int KEYMASTER_VERSION_INDEX = 2;
    static final int KEYMASTER_SECURITY_LEVEL_INDEX = 3;
    static final int ATTESTATION_CHALLENGE_INDEX = 4;
    static final int UNIQUE_ID_INDEX = 5;
    static final int SW_ENFORCED_INDEX = 6;
    static final int HW_ENFORCED_INDEX = 7;

    int attestationSecurityLevel;

    /**
     * Constructs an {@code Asn1Attestation} object from the provided {@link X509Certificate},
     * extracting the attestation data from the attestation extension.
     *
     * @throws CertificateParsingException if the certificate does not contain a properly-formatted
     *     attestation extension.
     */

    public Asn1Attestation(X509Certificate x509Cert) throws CertificateParsingException {
        // getAttestationSequence 方法中调用了 getExtensionValue 用于获取证书扩展部分的数据
        ASN1Sequence seq = getAttestationSequence(x509Cert);


        // 下方的字段来源于 KeyDescription ，具体如下：
//        KeyDescription ::= SEQUENCE {
//            attestationVersion  1,
//                    attestationSecurityLevel  SecurityLevel,
//                    keymasterVersion  INTEGER,
//                    keymasterSecurityLevel  SecurityLevel,
//                    attestationChallenge  OCTET_STRING,
//                    uniqueId  OCTET_STRING,
//                    softwareEnforced  AuthorizationList,
//                    hardwareEnforced  AuthorizationList,
//        }

        // 需要注意的是，在KeyAttestation项目中，hardwareEnforced 字段的位置写的是 teeEnforced
        // 这是因为 hardwareEnforced 由 TEE（Trusted Execution Environment）或者 StrongBox 硬件安全模块强制执行的约束
        // rootOfTrust 只会出现在 hardwareEnforced 中
        // 因为它描述的是设备启动链、锁定状态、verified boot 等安全根属性，而这些只能由硬件环境保证
        // 结论：想要分析 RootOfTrust 那么只需要分析 hardwareEnforced（teeEnforced）
        attestationVersion =
                Asn1Utils.getIntegerFromAsn1(seq.getObjectAt(ATTESTATION_VERSION_INDEX));
        attestationSecurityLevel =
                Asn1Utils.getIntegerFromAsn1(seq.getObjectAt(ATTESTATION_SECURITY_LEVEL_INDEX));
        keymasterVersion = Asn1Utils.getIntegerFromAsn1(seq.getObjectAt(KEYMASTER_VERSION_INDEX));
        keymasterSecurityLevel =
                Asn1Utils.getIntegerFromAsn1(seq.getObjectAt(KEYMASTER_SECURITY_LEVEL_INDEX));
        attestationChallenge =
                Asn1Utils.getByteArrayFromAsn1(seq.getObjectAt(ATTESTATION_CHALLENGE_INDEX));
        uniqueId = Asn1Utils.getByteArrayFromAsn1(seq.getObjectAt(UNIQUE_ID_INDEX));
        softwareEnforced = new AuthorizationList(seq.getObjectAt(SW_ENFORCED_INDEX));
        hardwareEnforced = new AuthorizationList(seq.getObjectAt(HW_ENFORCED_INDEX));
    }

    ASN1Sequence getAttestationSequence(X509Certificate x509Cert)
            throws CertificateParsingException {
        // 通过 getExtensionValue 获取证书扩展部分的数据
        byte[] attestationExtensionBytes = x509Cert.getExtensionValue(Attestation.ASN1_OID);
        if (attestationExtensionBytes == null || attestationExtensionBytes.length == 0) {
            throw new CertificateParsingException("Did not find extension with OID " + ASN1_OID);
        }
        return Asn1Utils.getAsn1SequenceFromBytes(attestationExtensionBytes);
    }

    public int getAttestationSecurityLevel() {
        return attestationSecurityLevel;
    }

    public RootOfTrust getRootOfTrust() {
        // 需要注意的是，一般情况下 softwareEnforced 中是没有 RootOfTrust 的
        // 所以优先获取的是 hardwareEnforced（也就是 teeEnforced） 中的 RootOfTrust ，其原因如下：
        // Keymaster 设备管理的每个密钥必须绑定到一个 root of trust——这是一个启动时（最好由 bootloader）提供给安全环境（TEE/StrongBox）的位字符串，并且必须对每个密钥进行加密绑定
        // 出自于https://android.googlesource.com/platform/hardware/interfaces/+/master/keymaster/4.0/IKeymasterDevice.hal
        RootOfTrust tee = hardwareEnforced.getRootOfTrust();
        if (tee != null) return tee;
        return softwareEnforced.getRootOfTrust();
    }
}
