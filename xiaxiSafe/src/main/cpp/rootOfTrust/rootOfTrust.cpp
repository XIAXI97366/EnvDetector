//
// Created by XIAXI on 2025/8/31.
//
#include "rootOfTrust.h"

jobject RootOfTrust::GetCertificateChain(JNIEnv *env, jobject keyAttestaion) {
    bool state = false;
    jobject objKeyStore = nullptr;
    jobjectArray objCertificateChain = nullptr;
    jstring alias = nullptr;
    jobject objCertificateFactory = nullptr;
    jsize chainLength = 0;
    jobject objCert = nullptr;
    jbyteArray encodedCert = nullptr;
    jbyteArray encodedCertChain = nullptr;
    jobject objOutStream = nullptr;
    jobject objInStream = nullptr;
    jobject objList = nullptr;
    jobject objCertChain = nullptr;
    jboolean result = JNI_FALSE;
    jboolean hasAlias = JNI_FALSE;

    // 1、获取 java 端的 com.xiaxi.safe.util.KeyAttestation.KeyAttestation 类中的 alias 字段
    state = invoke_func()->getObject(env, (jobject *)&alias, "com/xiaxi/safe/util/KeyAttestation/KeyAttestation",
                                     keyAttestaion, "alias", "Ljava/lang/String;");
    if (!state || nullptr == alias){
        LOGE("[-] %s %d invoke_func()->getObject faild ", __FUNCTION__ , __LINE__);
        return nullptr;
    }

    // 2、获取对应 AndroidKeyStore 的 jni 静态类对象 objKeyStore
    state = invoke_func()->callStaticObjectMethod(env, &objKeyStore, "java/security/KeyStore",
                                          "(Ljava/lang/String;)Ljava/security/KeyStore;", "getInstance",
                                          env->NewStringUTF("AndroidKeyStore"));
    if (!state || nullptr == objKeyStore){
        LOGE("[-] %s %d invoke_func()->callStaticObjectMethod faild ", __FUNCTION__ , __LINE__);
        return nullptr;
    }

    // 2.1、确保 KeyStore 处于已加载状态：AndroidKeyStore 需要先 load(null)
    state = invoke_func()->callVoidMethod(env, "java/security/KeyStore", objKeyStore,
                                          "(Ljava/security/KeyStore$LoadStoreParameter;)V",
                                          "load", nullptr);
    if (!state){
        LOGE("[-] %s %d invoke_func()->callVoidMethod faild ", __FUNCTION__ , __LINE__);
        return nullptr;
    }

    // 2.2、检查 alias 是否存在
    state = invoke_func()->callBooleanMethod(env, &hasAlias, "java/security/KeyStore", objKeyStore,
                                             "(Ljava/lang/String;)Z", "containsAlias", alias);
    if (!state || JNI_TRUE != hasAlias){
        LOGE("[-] %s %d alias not found in AndroidKeyStore ", __FUNCTION__ , __LINE__);
        return nullptr;
    }

    // 3、通过 objKeyStore 调用 getCertificateChain 方法传入 alias 参数获取对应的证书链（objCertificateChain）
    state = invoke_func()->callObjectMethod(env, (jobject *)&objCertificateChain, "java/security/KeyStore",
                                            objKeyStore, "(Ljava/lang/String;)[Ljava/security/cert/Certificate;",
                                            "getCertificateChain", alias);
    if (!state || nullptr == objCertificateChain){
        LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
        return nullptr;
    }

    // 4、获取可以处理 X.509 证书的 CertificateFactory 实例（objCertificateFactory），该对象可以用来解析 DER/PEM 编码的证书
    state = invoke_func()->callStaticObjectMethod(env, &objCertificateFactory, "java/security/cert/CertificateFactory",
                                          "(Ljava/lang/String;)Ljava/security/cert/CertificateFactory;", "getInstance",
                                          env->NewStringUTF("X.509"));
    if(!state || nullptr == objCertificateFactory){
        LOGE("[-] %s %d invoke_func()->callStaticObjectMethod faild ", __FUNCTION__ , __LINE__);
        return nullptr;
    }

    // 5、新建 ByteArrayOutputStream 的对象
    objOutStream = invoke_func()->newInstance(env, "java/io/ByteArrayOutputStream", "(I)V", 8192);
    if (nullptr == objOutStream){
        LOGE("[-] %s %d invoke_func()->newInstance faild ", __FUNCTION__ , __LINE__);
        return nullptr;
    }

    // 6、循环获取证书链中的每张证书，都经由 DER 编码，统一保存在 ByteArrayOutputStream 的对象中
    chainLength =  env->GetArrayLength(objCertificateChain);
    for (int i = 0; i < chainLength; ++i) {
        // 循环获取证书链中的证书
        objCert = env->GetObjectArrayElement(objCertificateChain, i);
        // 对循环获取的证书进行 DER 编码
        state = invoke_func()->callObjectMethod(env, (jobject *)&encodedCert, "java/security/cert/Certificate",
                                        objCert, "()[B", "getEncoded");
        if (!state || nullptr == encodedCert){
            LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
            return nullptr;
        }
        // 将经过 DER 编码的每张证书循环写入 ByteArrayOutputStream 对象中
        state = invoke_func()->callVoidMethod(env, "java/io/ByteArrayOutputStream", objOutStream, "([B)V",
                                      "write", encodedCert);
        if (!state){
            LOGE("[-] %s %d invoke_func()->callVoidMethod faild ", __FUNCTION__ , __LINE__);
            return nullptr;
        }
    }

    // 7、通过 objOutStream 调用 ByteArrayOutputStream 的 toByteArray 方法获取保存所有经由 DER 编码的证书
    invoke_func()->callObjectMethod(env, (jobject *)&encodedCertChain, "java/io/ByteArrayOutputStream",
                                    objOutStream, "()[B", "toByteArray");
    if (!state || nullptr == encodedCertChain){
        LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
        return nullptr;
    }

    // 8、用 encodedCertChain（所有经由 DER 编码的证书）新建 ByteArrayInputStream 类的对象（objInStream）
    objInStream = invoke_func()->newInstance(env, "java/io/ByteArrayInputStream", "([B)V", encodedCertChain);
    if (nullptr == objInStream){
        LOGE("[-] %s %d invoke_func()->newInstance faild ", __FUNCTION__ , __LINE__);
        return nullptr;
    }

    // 9、通过 objCertificateFactory 调用 generateCertificates 从输出流中（objInStream）获取一组 X.509 证书
    state = invoke_func()->callObjectMethod(env, &objList, "java/security/cert/CertificateFactory",
                                    objCertificateFactory, "(Ljava/io/InputStream;)Ljava/util/Collection;",
                                    "generateCertificates", objInStream);
    if (!state || nullptr == objList){
        LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
        return nullptr;
    }

    // 10、新建 ArrayList 的对象 objCertChain 用来保存 X.509 证书并返回
    objCertChain = invoke_func()->newInstance(env, "java/util/ArrayList", "()V");
    if (nullptr == objCertChain){
        LOGE("[-] %s %d invoke_func()->newInstance faild ", __FUNCTION__ , __LINE__);
        return nullptr;
    }
    state = invoke_func()->callBooleanMethod(env, &result, "java/util/ArrayList", objCertChain,
                                     "(Ljava/util/Collection;)Z", "addAll", objList);
    if (!state){
        LOGE("[-] %s %d invoke_func()->callBooleanMethod faild ", __FUNCTION__ , __LINE__);
        return nullptr;
    }

    return objCertChain;
}

bool RootOfTrust::ParseCertificateChain(JNIEnv *env, jobject objCertChain) {
    jobject objSortCerts = nullptr;
    jsize arrayCount = 0;
    jobject objX509Cert = nullptr;
    jbyteArray verifiedBootKey_EAT = nullptr;
    jboolean deviceLocked_EAT = JNI_FALSE;
    jint verifiedBootState_EAT = 0;
    bool state = false;

    // 1、调用 AttestationData 的 sortCerts 方法对 objCertChain（中的所有 X.509 证书）进行排序重新保存在 objSortCerts
    state = invoke_func()->callStaticObjectMethod(env, &objSortCerts, "com/xiaxi/safe/util/KeyAttestation/AttestationData",
                                          "(Ljava/util/List;)Ljava/util/List;", "sortCerts", objCertChain);
    if (!state || nullptr == objSortCerts){
        LOGE("[-] %s %d invoke_func()->callStaticObjectMethod faild ", __FUNCTION__ , __LINE__);
        return false;
    }

    // 2、获取 objSortCerts 中的 X.509 证书个数
    state = invoke_func()->callIntMethod(env, &arrayCount, "java/util/ArrayList", objSortCerts, "()I", "size");
    if (!state || 0 == arrayCount){
        LOGE("[-] %s %d invoke_func()->callIntMethod faild or  arrayCount == 0 ", __FUNCTION__ , __LINE__);
        return false;
    }

    // 3、循环获取单个 X.509 的证书，检查当前证书是否包含认证密钥信息（也就是检验当前证书是否具有 AttestationKey 特性）
    for (int i = arrayCount - 1; i >= 0; i--) {
        state = invoke_func()->callObjectMethod(env, &objX509Cert, "java/util/ArrayList", objSortCerts,
                                                "(I)Ljava/lang/Object;", "get", i);
        if (!state || nullptr == objX509Cert){
            LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
            return false;
        }

        // 返回 true：不包含认证密钥用途（purposes 为 null 或不含 KM_PURPOSE_ATTEST_KEY）（也就是当前证书是业务证书，停止遍历）
        // 返回 false：包含认证密钥用途（含 KM_PURPOSE_ATTEST_KEY）或解析失败抛异常的情况（当前证书是认证密钥证书（或未能解析），继续遍历）
        if (CheckAttestation(env, objX509Cert)) {
            break;
        }
    }

    // 需要区分 ASN1 和 EAT
    if (nullptr != objRootOfTrust_EAT){
        // 获取 verifiedBootKey deviceLocked verifiedBootState 这三个字段
        state = invoke_func()->callObjectMethod(env, (jobject *)&verifiedBootKey_EAT,
                                                "com/xiaxi/safe/util/KeyAttestation/RootOfTrust",
                                 objRootOfTrust_EAT, "()[B", "getVerifiedBootKey");
        if (!state){
            LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
            return false;
        }

        state = invoke_func()->callBooleanMethod(env, &deviceLocked_EAT,
                                                 "com/xiaxi/safe/util/KeyAttestation/RootOfTrust",
                                        objRootOfTrust_EAT, "()Z", "isDeviceLocked");
        if (!state){
            LOGE("[-] %s %d invoke_func()->callBooleanMethod faild ", __FUNCTION__ , __LINE__);
            return false;
        }

        state = invoke_func()->callIntMethod(env, &verifiedBootState_EAT,
                                             "com/xiaxi/safe/util/KeyAttestation/RootOfTrust",
                                         objRootOfTrust_EAT, "()I", "getVerifiedBootState");
        if (!state){
            LOGE("[-] %s %d invoke_func()->callIntMethod faild ", __FUNCTION__ , __LINE__);
            return false;
        }

        if (0 == env->GetArrayLength(verifiedBootKey_EAT) ||
            JNI_FALSE != deviceLocked_EAT ||
            RootOfTrust::KM_VERIFIED_BOOT_VERIFIED != verifiedBootState_EAT){
            // 设备已解锁
            LOGE("[+] %s %d Device Is Unlocked ", __FUNCTION__ , __LINE__);
            return true;
        }else{
            // 设备未解锁
            LOGE("[+] %s %d Device Is locked ", __FUNCTION__ , __LINE__);
            return true;
        }
    }else{
        // Software and Tee broken, return faild.
        if (nullptr == softwareEnforced_ASN1.get() && nullptr == teeEnforced_ASN1.get()) {
            LOGE("[-] %s %d softwareEnforced.get() and teeEnforced.get() is nullptr ", __FUNCTION__ , __LINE__);
            return false;
        }

        if (teeEnforced_ASN1.get() != nullptr && teeEnforced_ASN1->rootOfTrust != nullptr) {
            if(!teeEnforced_ASN1->rootOfTrust->IsDeviceLocked() ||
                teeEnforced_ASN1->rootOfTrust->GetVerifiedBootState() != RootOfTrust::KM_VERIFIED_BOOT_VERIFIED ||
                0 == env->GetArrayLength(verifiedBootKey)){
                // 设备已解锁
                LOGE("[+] %s %d Device Is Unlocked ", __FUNCTION__ , __LINE__);
                return true;
            }else{
                // 设备未解锁
                LOGE("[+] %s %d Device Is locked ", __FUNCTION__ , __LINE__);
                return false;
            }
        }

        // I assume that Software isn't as reliable as Tee so we only check that if tee returned locked.
        if (softwareEnforced_ASN1.get() != nullptr && softwareEnforced_ASN1->rootOfTrust != nullptr) {
            if(!softwareEnforced_ASN1->rootOfTrust->IsDeviceLocked() ||
                softwareEnforced_ASN1->rootOfTrust->GetVerifiedBootState() != RootOfTrust::KM_VERIFIED_BOOT_VERIFIED ||
                env->GetArrayLength(verifiedBootKey)){
                LOGE("[+] %s %d Device Is Unlocked ", __FUNCTION__ , __LINE__);
                return true;
            }else{
                LOGE("[+] %s %d Device Is locked ", __FUNCTION__ , __LINE__);
                return false;
            }
        }
    }
}

// 检查当前证书是否包含认证密钥信息（也就是检验当前证书是否具有 AttestationKey 特性）
    // 返回 true：不包含认证密钥用途（purposes 为 null 或不含 KM_PURPOSE_ATTEST_KEY）（也就是当前证书是业务证书，停止遍历）
    // 返回 false：包含认证密钥用途（含 KM_PURPOSE_ATTEST_KEY）或解析失败抛异常的情况（当前证书是认证密钥证书（或未能解析），继续遍历）
bool RootOfTrust::CheckAttestation(JNIEnv *env, jobject objX509Cert) {
    bool state = false;
    std::set<int> purposes_ANS1;
    jobject objPurposes_EAT = nullptr;
    jobject objKey = nullptr;
    jboolean has = JNI_FALSE;

    try{
        //尝试从当前 X509格式 的证书中加载认证信息（LoadFromCertificate） 并解析证书中的认证扩展数据
        state = LoadFromCertificate(env, objX509Cert);
        if (!state){
            return state;
        }else if (nullptr != objRootOfTrust_EAT){
            // 说当前 X509 证书走 EAT 扩展
            LOGE("[+] %s %d execute EAT ", __FUNCTION__ , __LINE__);
            // 需要获取 objSoftwareEnforced_EAT 或者 objTeeEnforced_EAT 中的 purposes 判断是否包含了 KM_PURPOSE_ATTEST_KEY
            if (nullptr != objTeeEnforced_EAT){
                // 优先选择 objTeeEnforced_EAT 如果为 objTeeEnforced_EAT 为 nullptr 那么再选择 objSoftwareEnforced_EAT
                state = invoke_func()->callObjectMethod(env, &objPurposes_EAT, "com/xiaxi/safe/util/KeyAttestation/AuthorizationList",
                                                        objTeeEnforced_EAT, "()Ljava/util/Set;", "getPurposes");
            }else{
                state = invoke_func()->callObjectMethod(env, &objPurposes_EAT, "com/xiaxi/safe/util/KeyAttestation/AuthorizationList",
                                                        objSoftwareEnforced_EAT, "()Ljava/util/Set;", "getPurposes");
            }
            if (!state || nullptr == objPurposes_EAT){
                LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
                return state;
            }

            state = invoke_func()->callStaticObjectMethod(env, &objKey, "java/lang/Integer", "(I)Ljava/lang/Integer;",
                                                  "valueOf", KM_PURPOSE_ATTEST_KEY);
            if (!state || nullptr == objKey){
                LOGE("[-] %s %d invoke_func()->callStaticObjectMethod faild ", __FUNCTION__ , __LINE__);
                return state;
            }

            state = invoke_func()->callBooleanMethod(env, &has, (const char *)env->GetObjectClass(objPurposes_EAT),
                                                     objPurposes_EAT, "(Ljava/lang/Object;)Z", "contains", objKey);
            if (!state){
                LOGE("[-] %s %d invoke_func()->callBooleanMethod faild ", __FUNCTION__ , __LINE__);
                return state;
            }
            return (has == JNI_FALSE) ? true : false;
        }else{
            // 当前 X509 证书走 ASN1 扩展
            LOGE("[+] %s %d execute ASN1 ", __FUNCTION__ , __LINE__);
            // teeEnforced->purposes.empty() 不是空返回 false 取反 true 拿 teeEnforced->purposes 反之获取 softwareEnforced_ASN1->purposes
            purposes_ANS1 = !teeEnforced_ASN1->purposes.empty() ? teeEnforced_ASN1->purposes : softwareEnforced_ASN1->purposes;
            // 如果 purposes 是空 → 返回 false
            // 如果 purposes 里 找不到 KM_PURPOSE_ATTEST_KEY → 返回 false
            // 其他情况（即集合非空且包含 KM_PURPOSE_ATTEST_KEY）→ 返回 true
            // 为 true：集合不为空，且找到了KM_PURPOSE_ATTEST_KEY（例如 Keymaster 用途“ATTEST_KEY”，常见值为7）。
            // 为 false：集合为空，或不包含该用途标识
            if (0 == purposes_ANS1.empty() || purposes_ANS1.find(KM_PURPOSE_ATTEST_KEY) == purposes_ANS1.end()){
                // 如果集合为空或者集合中找不到该元素则返回true，代表当前X509证书是业务证书，停止遍历
                return true;
            }else{
                // 反之集合不为空切包含认证密钥用途（含 KM_PURPOSE_ATTEST_KEY）或解析失败抛异常的情况（当前证书是认证密钥证书（或未能解析），继续遍历）
                return false;
            }
        }
    }catch (...){
        return false;
    }
}

bool RootOfTrust::LoadFromCertificate(JNIEnv *env, jobject objX509Cert) {
    bool state = false;
    jbyteArray extensionValue_ASN1 = nullptr;
    jbyteArray extensionValue_EAT = nullptr;
    jbyteArray extensionValue_KNOX = nullptr;
    jstring asn1Oid = env->NewStringUTF(RootOfTrust::ASN1_OID.c_str());
    jstring eatOid = env->NewStringUTF(RootOfTrust::EAT_OID.c_str());
    jstring knoxOid = env->NewStringUTF(RootOfTrust::KNOX_OID.c_str());

    // 1、调用 getExtensionValue 方法判断当前证书的扩展部分数据是 EAT 扩展还是 ASN.1 扩展
    state = invoke_func()->callObjectMethod(env, (jobject *)&extensionValue_ASN1, "java/security/cert/X509Certificate",
                                            objX509Cert, "(Ljava/lang/String;)[B", "getExtensionValue", asn1Oid);
    if (!state){
        LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
        return false;
    }
    state = invoke_func()->callObjectMethod(env, (jobject *)&extensionValue_EAT, "java/security/cert/X509Certificate",
                                            objX509Cert, "(Ljava/lang/String;)[B", "getExtensionValue", eatOid);
    if (!state){
        LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
        return false;
    }

    // 2、如果当前证书既不是 EAT 扩展也不是 ASN.1 扩展
    if (nullptr == extensionValue_ASN1 && nullptr == extensionValue_EAT){
        // 则查看当前证书是否为 KNOX 扩展
        state = invoke_func()->callObjectMethod(env, (jobject *)&extensionValue_KNOX, "java/security/cert/X509Certificate",
                                                objX509Cert, "(Ljava/lang/String;)[B", "getExtensionValue", knoxOid);
        if (!state){
            LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
            return false;
        }
        if (nullptr != extensionValue_KNOX) {
            // 如果当前证书属于 KNOX 扩展，则暂时不支持对该扩展的解析
            native_Asn1Utils:: ThrowException(env, "java/security/cert/CertificateParsingException",
                                              "Knox attestation extensions found，Not supported yet");
        }else{
            // 如果当前证书三个扩展都不支持，那么当前证书则无扩展或未知扩展
            native_Asn1Utils:: ThrowException(env, "java/security/cert/CertificateParsingException",
                                              "No attestation extensions found");
        }
        return false;
    }

    // 如果证书既有 EAT 扩展也有 ASN.1 扩展，则该证书无效
    // （同一张证明证书里，EAT 扩展和 ASN.1 扩展是两种"等价的证明载体/编码"，不能同时出现，强制二选一）
    if (nullptr != extensionValue_EAT){
        if (nullptr != extensionValue_ASN1){
            native_Asn1Utils::ThrowException(env, "java/security/cert/CertificateParsingException",
                                              "Multiple attestation extensions found");
            return false;
        }

        // 证书中只有 EAT 扩展并对该扩展进行解析，直接获取 EatAttestation 对象中的 rootOfTrust 字段
        state = EatAttestation(env, objX509Cert);
        return state;
    }

    // 证书中只有 ANS1 扩展并对该扩展进行解析
    state = Asn1Attestation(env, objX509Cert);
    return state;
}

bool RootOfTrust::Asn1Attestation(JNIEnv* env, jobject objX509Cert) {
    auto GetObjectAt = [env](jobject sequence, int index) -> jobject {
        jclass sequenceClass = env->FindClass("org/bouncycastle/asn1/ASN1Sequence");
        jmethodID getObjectAtMethod = env->GetMethodID(sequenceClass, "getObjectAt",
                                                       "(I)Lorg/bouncycastle/asn1/ASN1Encodable;");
        return env->CallObjectMethod(sequence, getObjectAtMethod, index);
    };

    jobject objSequence = nullptr;
    jobject objSoftwareEnforced = nullptr;
    jobject objTeeEnforced = nullptr;
    bool state = false;

    state = GetAttestationSequence(env, objX509Cert, &objSequence);
    if (!state){
        LOGE("[-] %s %d GetAttestationSequence faild ", __FUNCTION__ , __LINE__);
        return false;
    }

    // objSoftwareEnforced 和 objTeeEnforced 都是 AuthorizationList 类型
    objSoftwareEnforced = GetObjectAt(objSequence, SW_ENFORCED_INDEX);
    if (nullptr == objSoftwareEnforced){
        LOGE("[-] %s %d GetObjectAt faild ", __FUNCTION__ , __LINE__);
        return false;
    }
    softwareEnforced_ASN1 = std::make_unique<Attest>(env, objSoftwareEnforced, this);

    objTeeEnforced = GetObjectAt(objSequence, TEE_ENFORCED_INDEX);
    if (nullptr == objTeeEnforced){
        LOGE("[-] %s %d GetObjectAt faild ", __FUNCTION__ , __LINE__);
        return false;
    }
    teeEnforced_ASN1 = std::make_unique<Attest>(env, objTeeEnforced, this);

    return state;
}

bool RootOfTrust::GetAttestationSequence(JNIEnv* env, jobject objX509Cert, jobject *objResult) {
    bool state = false;
    jsize arraySize = 0;
    jbyteArray attestationExtensionBytes = nullptr;

    // 1、获取 X509 证书扩展部分的数据
    state = invoke_func()->callObjectMethod(env, (jobject *)&attestationExtensionBytes, "java/security/cert/X509Certificate",
                                            objX509Cert, "(Ljava/lang/String;)[B", "getExtensionValue",
                                    env->NewStringUTF(ASN1_OID.c_str()));
    if(!state || nullptr == attestationExtensionBytes){
        LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
        return state;
    }

    arraySize = env->GetArrayLength(attestationExtensionBytes);
    if (0 == arraySize){
        native_Asn1Utils:: ThrowException(env, "java/lang/IllegalArgumentException",
                                          "Expected ASN1TaggedObject");
        return false;
    }

    *objResult = native_Asn1Utils::GetAsn1SequenceFromBytes(env, attestationExtensionBytes);
    if (nullptr == *objResult){
        LOGE("[-] %s %d native_Asn1Utils::GetAsn1SequenceFromBytes faild ", __FUNCTION__ , __LINE__);
        return false;
    }

    return true;
}

// 该函数其目的还是用来获取 ASN1Sequence 中的每个元素
bool RootOfTrust::ParseAsn1TaggedObject(JNIEnv* env, jobject objParser, jobject *objResult){
    bool state = false;
    jclass clsASN1TaggedObject = nullptr;

    // 通过迭代器调用 readObject 逐个读取 parser 对象中的元素
    state = invoke_func()->callObjectMethod(env, objResult, "org/bouncycastle/asn1/ASN1SequenceParser",
                                            objParser, "()Lorg/bouncycastle/asn1/ASN1Encodable;",
                                            "readObject");
    if (!state){
        LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
        return state;
    }

    clsASN1TaggedObject = env->FindClass("org/bouncycastle/asn1/ASN1TaggedObject");
    if (nullptr == clsASN1TaggedObject){
        LOGE("[-] %s %d clsASN1TaggedObject is nullptr ", __FUNCTION__ , __LINE__);
        return false;
    }

    if (env->IsInstanceOf(*objResult, clsASN1TaggedObject)){
        state = true;
    }else{
        native_Asn1Utils::ThrowException(env, "java/lang/IllegalArgumentException",
                                         "Expected ASN1TaggedObject");
        return false;
    }

    return state;
}


bool RootOfTrust::EatAttestation(JNIEnv* env, jobject objX509Cert){
    bool state = false;
    jobject objEatAttestation = nullptr;

    // 调用 java 端的 EatAttestation 的构造对该 X509 证书的 EAT 扩展进行解析
    objEatAttestation = invoke_func()->newInstance(env, "com/xiaxi/safe/util/KeyAttestation/EatAttestation",
                                                   "(Ljava/security/cert/X509Certificate;)V", objX509Cert);
    if (nullptr == objEatAttestation){
        LOGE("[-] %s %d invoke_func()->newInstance faild ", __FUNCTION__ , __LINE__);
        return false;
    }

    // 获取 EatAttestation 对象中的 rootOfTrust 字段
    if (nullptr != objRootOfTrust_EAT){
        env->DeleteGlobalRef(objRootOfTrust_EAT);
        objRootOfTrust_EAT = nullptr;
    }
    state = invoke_func()->getObject(env, &objRootOfTrust_EAT, "com/xiaxi/safe/util/KeyAttestation/EatAttestation",
                                     objEatAttestation, "rootOfTrust",
                                     "Lcom/xiaxi/safe/util/KeyAttestation/RootOfTrust;");
    if (!state || nullptr == objRootOfTrust_EAT){
        LOGE("[-] %s %d invoke_func()->getObject faild ", __FUNCTION__ , __LINE__);
        return state;
    }

    // 获取 EatAttestation 对象中的 softwareEnforced 和 teeEnforced 字段
    if (nullptr != objSoftwareEnforced_EAT){
        env->DeleteGlobalRef(objSoftwareEnforced_EAT);
        objSoftwareEnforced_EAT = nullptr;
    }
    state = invoke_func()->getObject(env, &objSoftwareEnforced_EAT, "com/xiaxi/safe/util/KeyAttestation/EatAttestation",
                                     objEatAttestation, "softwareEnforced",
                                     "Lcom/xiaxi/safe/util/KeyAttestation/AuthorizationList;");
    if (!state){
        LOGE("[-] %s %d invoke_func()->getObject faild ", __FUNCTION__ , __LINE__);
        return state;
    }
    if (nullptr != objTeeEnforced_EAT){
        env->DeleteGlobalRef(objTeeEnforced_EAT);
        objTeeEnforced_EAT = nullptr;
    }
    state = invoke_func()->getObject(env, &objTeeEnforced_EAT, "com/xiaxi/safe/util/KeyAttestation/EatAttestation",
                                     objEatAttestation, "hardwareEnforced",
                                     "Lcom/xiaxi/safe/util/KeyAttestation/AuthorizationList;");
    if (!state){
        LOGE("[-] %s %d invoke_func()->getObject faild ", __FUNCTION__ , __LINE__);
        return state;
    }

    return state;
}

