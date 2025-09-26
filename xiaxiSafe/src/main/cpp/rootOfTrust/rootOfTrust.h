//
// Created by XIAXI on 2025/7/17.
//

#ifndef CHECKROM_ROOTOFTRUST_H
#define CHECKROM_ROOTOFTRUST_H

#include "../util/helper/helper.h"
#include "native_Asn1Utils.hpp"

class RootOfTrust {
public:
    class Attest {
    public:
        std::set<int> purposes;
        RootOfTrust *rootOfTrust;   // 外部类的指针
    public:
        Attest(JNIEnv* env, jobject objSequence, RootOfTrust *rootOfTrust) : rootOfTrust(rootOfTrust) {
            bool state = false;
            int tagNo = 0;
            jobject objParser = nullptr;
            jobject objEntry = nullptr;
            jobject objValue = nullptr;
            jobject objBase = nullptr;
            jclass clsASN1Sequence = env->FindClass("org/bouncycastle/asn1/ASN1Sequence");

            if (!env->IsInstanceOf(objSequence, clsASN1Sequence)) {
                native_Asn1Utils::ThrowException(env, "java/lang/IllegalArgumentException",
                                                 "Expected sequence for authorization list");
                return;
            }

            // 为当前 ASN1Sequence 创建一个迭代器（parser 对象），允许调用者逐个读取其中的元素保存在 objEntry 中
            state = invoke_func()->callObjectMethod(env, &objParser, "org/bouncycastle/asn1/ASN1Sequence",
                                            objSequence, "()Lorg/bouncycastle/asn1/ASN1SequenceParser;",
                                            "parser");
            if (!state || nullptr == objParser){
                LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
                return;
            }
            state = ParseAsn1TaggedObject(env, objParser, &objEntry);
            if (!state || nullptr == objEntry){
                LOGE("[-] %s %d ParseAsn1TaggedObject faild ", __FUNCTION__ , __LINE__);
                return;
            }

            // 当前项目只需要 purposes 和 rootOfTrust
            while(nullptr != objEntry){
                state = invoke_func()->callIntMethod(env, &tagNo, "org/bouncycastle/asn1/ASN1TaggedObject",
                                             objEntry, "()I", "getTagNo");
                if (!state){
                    LOGE("[-] %s %d invoke_func()->callIntMethod faild ", __FUNCTION__ , __LINE__);
                    return;
                }

                state = invoke_func()->callObjectMethod(env, &objBase, "org/bouncycastle/asn1/ASN1TaggedObject",
                                                        objEntry, "()Lorg/bouncycastle/asn1/ASN1Object;",
                                                        "getBaseObject");
                if (!state || nullptr == objBase){
                    LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
                    return;
                }

                state = invoke_func()->callObjectMethod(env, &objValue, "org/bouncycastle/asn1/ASN1Object",
                                                objBase, "()Lorg/bouncycastle/asn1/ASN1Primitive;",
                                                "toASN1Primitive");
                if (!state || nullptr == objValue){
                    LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
                    return;
                }

                switch (tagNo) {
                    case KM_TAG_PURPOSE & KEYMASTER_TAG_TYPE_MASK:
                        purposes = native_Asn1Utils::GetIntegersFromAsn1Set(env, objValue);
                        break;
                    case KM_TAG_ROOT_OF_TRUST & KEYMASTER_TAG_TYPE_MASK:
                        ParseASN1Sequence(env, objValue);
                        break;
                }

                // 获取 ASN1Sequence 中的下一个元素
                state = ParseAsn1TaggedObject(env, objParser, &objEntry);
                if(!state){
                    LOGE("[-] %s %d ParseAsn1TaggedObject faild ", __FUNCTION__ , __LINE__);
                    return;
                }
            }
        }

        void ParseASN1Sequence(JNIEnv *env, jobject objASN1Encodable) {
            jclass clsASN1Sequence = env->FindClass("org/bouncycastle/asn1/ASN1Sequence");
            jmethodID getObjectAtMethod = env->GetMethodID(clsASN1Sequence, "getObjectAt",
                                                           "(I)Lorg/bouncycastle/asn1/ASN1Encodable;");

            if (!env->IsInstanceOf(objASN1Encodable, clsASN1Sequence)) {
                native_Asn1Utils::ThrowException(env, "java/lang/IllegalArgumentException",
                                                 "Expected sequence for authorization list");
                return;
            }

            rootOfTrust->verifiedBootKey = native_Asn1Utils::GetByteArrayFromAsn1(env,env->CallObjectMethod(
                    objASN1Encodable, getObjectAtMethod, VERIFIED_BOOT_KEY_INDEX));
            rootOfTrust->deviceLocked = native_Asn1Utils::GetBooleanFromAsn1(env, env->CallObjectMethod(
                    objASN1Encodable, getObjectAtMethod, DEVICE_LOCKED_INDEX));
            rootOfTrust->verifiedBootState = (VerifiedBootState) native_Asn1Utils::GetIntegerFromAsn1(env, env->CallObjectMethod(
                    objASN1Encodable, getObjectAtMethod, VERIFIED_BOOT_STATE_INDEX));
        }
    };

public:
    enum VerifiedBootState {
        KM_VERIFIED_BOOT_VERIFIED = 0,
        KM_VERIFIED_BOOT_SELF_SIGNED = 1,
        KM_VERIFIED_BOOT_UNVERIFIED = 2,
        KM_VERIFIED_BOOT_FAILED = 3,
    };

    RootOfTrust(){}

    int GetVerifiedBootState() const {
        return verifiedBootState;
    }

    std::string GetVerifiedBootStateString() const {
        switch (verifiedBootState) {
            case VerifiedBootState::KM_VERIFIED_BOOT_VERIFIED: return "Verified";
            case VerifiedBootState::KM_VERIFIED_BOOT_SELF_SIGNED: return "Self Signed";
            case VerifiedBootState::KM_VERIFIED_BOOT_UNVERIFIED: return "Unverified";
            case VerifiedBootState::KM_VERIFIED_BOOT_FAILED: return "Failed";
        }
    }

    bool IsDeviceLocked() const {
        return deviceLocked;
    }

    int GetVerifiedBootState() {
        return verifiedBootState;
    }

    std::string GetVerifiedBootStateString() {
        switch (verifiedBootState) {
            case VerifiedBootState::KM_VERIFIED_BOOT_VERIFIED: return "Verified";
            case VerifiedBootState::KM_VERIFIED_BOOT_SELF_SIGNED: return "Self Signed";
            case VerifiedBootState::KM_VERIFIED_BOOT_UNVERIFIED: return "Unverified";
            case VerifiedBootState::KM_VERIFIED_BOOT_FAILED: return "Failed";
        }
    }

    /**
     * 获取已经 com.xiaxi.safe.util.KeyAttestation doAttestation 中生成的证书链
     * @param env
     * @param keyAttestaion
     */
    jobject GetCertificateChain(JNIEnv *env, jobject keyAttestaion);

    bool ParseCertificateChain(JNIEnv *env, jobject objCertChain);

    bool CheckAttestation(JNIEnv *env, jobject objX509Cert);

    bool LoadFromCertificate(JNIEnv *env, jobject objX509Cert);

    bool Asn1Attestation(JNIEnv *env, jobject objX509Cert);

    bool GetAttestationSequence(JNIEnv *env, jobject objX509Cert, jobject *objResult);

    bool static ParseAsn1TaggedObject(JNIEnv *env, jobject objParser, jobject *objResult);

    bool EatAttestation(JNIEnv* env, jobject objX509Cert);
public:
    static constexpr int KM_BYTES = 9 << 28;
    static constexpr int KM_ENUM_REP = 2 << 28;
    static constexpr int KM_PURPOSE_ATTEST_KEY = 7;
    static constexpr int KM_TAG_ROOT_OF_TRUST = KM_BYTES | 704;
    static constexpr int KM_TAG_PURPOSE = KM_ENUM_REP | 1;
    static constexpr int KEYMASTER_TAG_TYPE_MASK = 0x0FFFFFFF;
    static constexpr int ATTESTATION_CHALLENGE_INDEX = 4;
    static constexpr int SW_ENFORCED_INDEX = 6;
    static constexpr int TEE_ENFORCED_INDEX = 7;

    static constexpr int VERIFIED_BOOT_KEY_INDEX = 0;
    static constexpr int DEVICE_LOCKED_INDEX = 1;
    static constexpr int VERIFIED_BOOT_STATE_INDEX = 2;
    static constexpr int VERIFIED_BOOT_HASH_INDEX = 3;

    inline static std::string ASN1_OID = "1.3.6.1.4.1.11129.2.1.17";
    inline static std::string EAT_OID = "1.3.6.1.4.1.11129.2.1.25";
    inline static std::string KNOX_OID = "1.3.6.1.4.1.236.11.3.23.7";

    std::unique_ptr<Attest> softwareEnforced_ASN1 = nullptr;
    std::unique_ptr<Attest> teeEnforced_ASN1 = nullptr;

    jobject objRootOfTrust_EAT = nullptr;
    jobject objSoftwareEnforced_EAT = nullptr;
    jobject objTeeEnforced_EAT = nullptr;

    jbyteArray verifiedBootKey = nullptr;   // 如果bl解锁那么verifiedBootKey设置为空
    bool deviceLocked = true;               // 如果bl解锁那么该值为flase，如果未解锁则为true
    VerifiedBootState verifiedBootState;    // 如果bl解锁，那么该值则为Unverified同时也会将verifiedBootKey设置为空，如果未解锁则是Verified
};


#endif //CHECKROM_ROOTOFTRUST_H
