package com.xiaxi.safe.util.KeyAttestation;

import static com.xiaxi.safe.util.KeyAttestation.Attestation.KM_SECURITY_LEVEL_SOFTWARE;
import static com.xiaxi.safe.util.KeyAttestation.AttestationException.CODE_CANT_PARSE_CERT;

import android.util.Log;

import com.xx.shell.RiskCheckApplication;

import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

/**
 * 该类用于解析证书链的证书
 */
public class AttestationData {
    private boolean sw;
    public Attestation showAttestation;

    private RootOfTrust rootOfTrust;

    public boolean isSoftwareLevel() {
        return sw;
    }

    public RootOfTrust getRootOfTrust() {
        return rootOfTrust;
    }
    private AttestationData(List<CertificateInfo> certs) {
        // 如果证书链为空，直接返回 null
        if (certs.isEmpty()) {
            Log.e(RiskCheckApplication.TAG, "certs is Empty");
            return;
        }

        // 获取证书链最后一张的业务证书
        var info = certs.get(certs.size() - 1);
        var attestation = info.getAttestation();
        if (attestation != null) {
            this.showAttestation = attestation;
            rootOfTrust = attestation.getRootOfTrust();
            // 如果 attestation.getAttestationSecurityLevel() 等于软件安全级别常量 KM_SECURITY_LEVEL_SOFTWARE（0），则将布尔标记 sw 设为true
            // 表示本次证明非硬件背书（非 TEE/StrongBox）；否则为 false
            this.sw = attestation.getAttestationSecurityLevel() == KM_SECURITY_LEVEL_SOFTWARE;
        } else {
            throw new AttestationException(CODE_CANT_PARSE_CERT, info.getCertException());
        }
    }

    /**
     * 该函数用于对证书链中的证书进行排序（叶子 → 中间 → 根）
     * @param certs
     * @return
     */
    private static List<X509Certificate> sortCerts(List<X509Certificate> certs) {
        // 证书链中少于两张证书无需排序，直接返回原列表
        if (certs.size() < 2) {
            return certs;
        }

        var issuer = certs.get(0).getIssuerX500Principal();     // 取第一张证书的颁发者也就是（签发证书者）
        boolean okay = true;
        for (var cert : certs) {    // 开始遍历证书链中的证书
            var subject = cert.getSubjectX500Principal();   // 获取当前证书的主体（也就是证书的持有者）
            if (issuer.equals(subject)) {   // 若证书的颁发者等于当前证书持有者
                issuer = subject;   // 则更新 issuer 为当前主体，继续检查下一张（以此类推）
            } else {
                okay = false;       // 如果不匹配，则直接退出遍历证书链
                break;
            }
        }
        if (okay) {
            return certs;
            // 所以上述 for 循环的代码就是为了查看传入的证书列表是否已按链顺序排好
            // 若判定已排好就直接返回，未排好再走下方代码重新排序证书链
        }

        var newList = new ArrayList<X509Certificate>(certs.size());     // newList 最终用于存放排序好的证书链
        for (var cert : certs) {
            boolean found = false;
            var subject = cert.getSubjectX500Principal();   // 获取当前证书的主体（也就是证书的持有者）
            for (var c : certs) {
                if (c == cert) continue;    // 跳过本次循环，避免遍历当前证书
                if (c.getIssuerX500Principal().equals(subject)) {
                    // 若 c 的证书的颁发者等于当前证书的持有者，说明 cert 是上级/签发者（有子证书）便不是链的起点，则退出内层循环
                    found = true;
                    break;
                }
            }
            if (!found) {
                // 没有证书把它当作颁发者（found == false 时，也就是证书的颁布者不等于证书的持有人），将其认定为叶子，放入新链起点
                // 若遍历完都没找到这样的 c，（也就是说明没有任何证书由 c 签发）,即它不作为任何证书的 issuer，被视为叶子证书，加入 newList
                newList.add(cert);
            }
        }
        if (newList.size() != 1) {
            // 若未找到唯一叶子（0 个或多个），则无法确定唯一链起点，放弃排序，返回原列表
            // 这里的“叶子证书”指“终端实体证书（End-Entity/Leaf）”，它不是其它证书的签发者，不作为任何证书的 issuer，通常是链的开始（排序方向为 叶子 → 中间 → 根）
            // 与“中间/根”相对：中间/根证书会作为其它证书的 issuer，被别的证书引用。
            return certs;
        }

        var oldList = new LinkedList<>(certs);  // 将原列表复制为可修改队列
        oldList.remove(newList.get(0));     // 从剩余集合中移除已确定的叶子
        for (int i = 0; i < newList.size(); i++) {
            issuer = newList.get(i).getIssuerX500Principal();   // 获取当前证书的颁布者
            for (var it = oldList.iterator(); it.hasNext(); ) {
                var cert = it.next();   // 取下一张候选证书
                if (cert.getSubjectX500Principal().equals(issuer)) { // 如果外层循环的当前证书的 issuer 等于内层循环的当前证书的持有者
                    newList.add(cert);  // 追加到链尾（形成下一环）
                    it.remove();    // 从剩余集合中移除，避免重复使用
                    break;
                }
            }
        }
        if (!oldList.isEmpty()) {
            return certs;
        }
        return newList;
    }

    /**
     * 解析证书链中的证书，并将数据保存在 infoList 中
     * @param certs
     * @return
     */
    public static AttestationData parseCertificateChain(List<X509Certificate> certs) {
        var infoList = new ArrayList<CertificateInfo>(certs.size());
        CertificateInfo.parse(sortCerts(certs), infoList);
        return new AttestationData(infoList);
    }
}
