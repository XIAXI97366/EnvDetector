package com.xiaxi.safe.util.KeyAttestation;

import android.util.Log;

import com.xx.shell.RiskCheckApplication;

import java.security.cert.CertificateParsingException;
import java.security.cert.X509Certificate;
import java.util.List;

public class CertificateInfo {
    private final X509Certificate cert;
    public static final int KM_PURPOSE_ATTEST_KEY = 7;
    private Attestation attestation;
    private CertificateParsingException certException;
    private CertificateInfo(X509Certificate cert) {
        this.cert = cert;
    }

    public Attestation getAttestation() {
        return attestation;
    }

    public CertificateParsingException getCertException() {
        return certException;
    }

    /**
     * 检查证书中的认证信息并验证其安全性
     * 该函数用于确定证书链遍历是否应该终止
     *
     * @return true 表示应该终止遍历，false 表示继续遍历
     */
    private boolean checkAttestation() {
        boolean terminate = false;

        try {
            // 尝试从当前证书中加载认证信息 loadFromCertificate(cert) 并解析证书中的认证扩展字段
            // 同时区分当前测试机的扩展数据类型是 KeyMint 还是 Keymaster
            attestation = Attestation.loadFromCertificate(cert);

            // 关键安全验证逻辑：
            // 如果密钥用途包含 KeyPurpose::SIGN（签名用途）
            // 那么该密钥就可以用来签名任意数据，包括任何 tbsCertificate
            // 因此由该密钥产生的认证将没有任何安全属性
            // 只有当父证书能够证明密钥用途仅为 KeyPurpose::ATTEST_KEY（认证密钥）时，子证书才能被信任

            // 获取TEE（可信执行环境）强制执行的密钥用途列表
            var purposes = attestation.getTeeEnforced().getPurposes();

            // 判断是否应该终止遍历：
            // 1. 如果没有用途信息（purposes == null）
            // 2. 或者用途不包含 KM_PURPOSE_ATTEST_KEY（认证密钥用途）（位于：android.hardware.security.keymint.KeyPurpose 中，值 == 7）
            // 则终止遍历，因为找到了不安全的认证密钥
            terminate = purposes == null || !purposes.contains(KM_PURPOSE_ATTEST_KEY);
        } catch (CertificateParsingException e) {
            // 如果解析认证信息失败，弹出异常信息
            certException = e;
            Log.e(RiskCheckApplication.TAG, "checkAttestation", e);
            terminate = false; // 继续遍历，寻找其他可能的认证信息
        }

        return terminate;
    }

    /**
     * 静态方法：解析证书链并构建证书信息列表
     * 从根证书开始，自底向上遍历证书链，验证每个证书是否包含认证密钥信息
     *
     * @param certs 原始X509证书列表，按从叶子证书到根证书的顺序排列
     * @param infoList 输出参数，用于存储解析后的证书信息对象列表
     */
    public static void parse(List<X509Certificate> certs, List<CertificateInfo> infoList) {
        // 从证书链的末尾（根证书）开始，向前遍历到开头（叶子证书）
        for (int i = certs.size() - 1; i >= 0; i--) {
            var info = new CertificateInfo(certs.get(i));
            infoList.add(info);
            // 检查当前证书是否包含认证密钥信息（也就是检验当前证书是否具有 AttestationKey 特性）
            if (info.checkAttestation()) {
                // 返回 true：不包含认证密钥用途（purposes 为 null 或不含 KM_PURPOSE_ATTEST_KEY）（也就是当前证书是业务证书，停止遍历）
                // 返回 false：包含认证密钥用途（含 KM_PURPOSE_ATTEST_KEY），或解析失败抛异常的情况（当前证书是认证密钥证书（或未能解析），继续遍历）
                break;
            }
        }
    }
}