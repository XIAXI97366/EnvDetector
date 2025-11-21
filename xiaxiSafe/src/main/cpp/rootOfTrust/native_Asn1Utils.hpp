//
// Created by XIAXI on 2025/8/25.
//

#ifndef NATIVE_ASN1UTILS_HPP
#define NATIVE_ASN1UTILS_HPP

#include "../util/helper/helper.h"

namespace native_Asn1Utils{
    inline void ThrowException(JNIEnv *env, const char *clazz, const char *info) {
        env->ThrowNew(env->FindClass(clazz), info);
    }

    inline void ThrowExceptionDetail(JNIEnv *env, jobject obj, const char *clazz, std::string msg, const char *detail){
        jobject objClass = nullptr;
        jstring clsName = nullptr;
        const char *className = nullptr;

        objClass = env->GetObjectClass(obj);
        invoke_func()->callObjectMethod(env, (jobject *)&clsName, "java/lang/Class", objClass,
                                        "()Ljava/lang/String;", "getName");
        className = env->GetStringUTFChars(clsName, nullptr);
        msg = msg + className + detail;
        ThrowException(env, clazz, msg.c_str());
        env->ReleaseStringUTFChars(clsName, className);
    }

    inline jint BigIntegerToInt(JNIEnv *env, jobject bigInt){
        jint result = 0;
        bool state = false;

        state = invoke_func()->callIntMethod(env, &result, "java/math/BigInteger", bigInt, "()I", "intValue");
        if (!state){
            LOGE("[-] %s %d invoke_func()->callIntMethod faild ", __FUNCTION__ , __LINE__);
            return 0;
        }

        return result;
    }

    inline jlong BigIntegerToLong(JNIEnv *env, jobject bigInt){
        bool state = false;
        jobject objMaxValue = nullptr;
        jobject objZERO = nullptr;
        jobject objClass = nullptr;
        jint cmpMax = 0;
        jint cmpZero = 0;
        jstring clsName = nullptr;
        const char *className = nullptr;
        jlong result = 0;

        state = invoke_func()->callStaticObjectMethod(env, &objMaxValue, "java/math/BigInteger",
                                                      "(J)Ljava/math/BigInteger;","valueOf", (jlong)LLONG_MAX);
        if (!state){
            LOGE("[-] %s %d invoke_func()->callStaticObjectMethod faild ", __FUNCTION__ , __LINE__);
            return 0;
        }

        state = invoke_func()->getStaticObject(env, &objZERO, "java/math/BigInteger", "ZERO",
                                               "Ljava/math/BigInteger;");
        if (!state){
            LOGE("[-] %s %d invoke_func()->getStaticObject faild ", __FUNCTION__ , __LINE__);
            return 0;
        }

        objClass = env->GetObjectClass(bigInt);
        state = invoke_func()->callObjectMethod(env, (jobject *)&clsName, "java/lang/Class", objClass,
                                                "()Ljava/lang/String;", "getName");
        if (!state){
            LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
            return 0;
        }

        className = env->GetStringUTFChars(clsName, nullptr);
        state = invoke_func()->callIntMethod(env, &cmpMax, className, bigInt, "(Ljava/math/BigInteger;)I",
                                             "compareTo", objMaxValue);
        if (!state){
            LOGE("[-] %s %d invoke_func()->callIntMethod faild ", __FUNCTION__ , __LINE__);
            env->ReleaseStringUTFChars(clsName, className);
            return 0;
        }

        state = invoke_func()->callIntMethod(env, &cmpZero, className, bigInt, "(Ljava/math/BigInteger;)I",
                                             "compareTo", objZERO);
        if (!state){
            LOGE("[-] %s %d invoke_func()->callIntMethod faild ", __FUNCTION__ , __LINE__);
            env->ReleaseStringUTFChars(clsName, className);
            return 0;
        }

        if (cmpMax > 0 || cmpZero < 0) {
            ThrowException(env, "java/security/cert/CertificateParsingException", "INTEGER out of bounds");
            env->ReleaseStringUTFChars(clsName, className);
            return 0;
        }

        state = invoke_func()->callLongMethod(env, &result, className, bigInt, "()J", "longValue");
        if (!state){
            LOGE("[-] %s %d invoke_func()->callLongMethod faild ", __FUNCTION__ , __LINE__);
            env->ReleaseStringUTFChars(clsName, className);
            return 0;
        }

        env->ReleaseStringUTFChars(clsName, className);
        return result;
    }

    inline jint GetIntegerFromAsn1(JNIEnv *env, jobject asn1Encodable){
        jclass clsASN1Integer = nullptr;
        jclass clsASN1Enumerated = nullptr;
        jobject objValue = nullptr;
        bool state = false;
        std::string msg = "Integer value expected, ";

        clsASN1Integer = env->FindClass("org/bouncycastle/asn1/ASN1Integer");
        clsASN1Enumerated = env->FindClass("org/bouncycastle/asn1/ASN1Enumerated");
        if(nullptr == clsASN1Enumerated || nullptr == clsASN1Integer || env->ExceptionCheck()){
            env->ExceptionDescribe();
            env->ExceptionClear();
            return 0;
        }

        if (env->IsInstanceOf(asn1Encodable, clsASN1Integer)){
            state = invoke_func()->callObjectMethod(env, &objValue, "org/bouncycastle/asn1/ASN1Integer",
                                                    asn1Encodable,"()Ljava/math/BigInteger;", "getValue");
            if (!state){
                LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
                return 0;
            }
            return BigIntegerToInt(env, objValue);
        }else if(env->IsInstanceOf(asn1Encodable, clsASN1Enumerated)){
            state = invoke_func()->callObjectMethod(env, &objValue, "org/bouncycastle/asn1/ASN1Enumerated",
                                                    asn1Encodable, "()Ljava/math/BigInteger;", "getValue");
            if (!state){
                LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
                return 0;
            }
            return BigIntegerToInt(env, objValue);
        }else{
            ThrowExceptionDetail(env, asn1Encodable, "java/security/cert/CertificateParsingException",
                                 msg, " found.");
        }

        return 0;
    }

    inline jlong GetLongFromAsn1(JNIEnv *env, jobject asn1Encodable){
        jobject objBigInt = nullptr;
        jclass clsASN1Integer = nullptr;
        bool state = false;
        jlong longValue = 0;

        clsASN1Integer = env->FindClass("org/bouncycastle/asn1/ASN1Integer");
        if (env->IsInstanceOf(asn1Encodable, clsASN1Integer)) {
            state = invoke_func()->callObjectMethod(env, &objBigInt, "org/bouncycastle/asn1/ASN1Integer", asn1Encodable,
                                            "()Ljava/math/BigInteger;", "getValue");
            if (!state){
                LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
                return 0;
            }

            longValue = BigIntegerToLong(env, objBigInt);
            return longValue;
        }else{
            ThrowException(env, "java/security/cert/CertificateParsingException",
                           "Expected ASN1Integer");
        }

        return 0;
    }

    inline jbyteArray GetByteArrayFromAsn1(JNIEnv *env, jobject asn1Encodable){
        jclass clsDEROctetString = nullptr;
        jbyteArray result = nullptr;
        bool state = false;

        clsDEROctetString = env->FindClass("org/bouncycastle/asn1/DEROctetString");
        if (env->IsInstanceOf(asn1Encodable, clsDEROctetString)) {
            state = invoke_func()->callObjectMethod(env, (jobject *)&result, "org/bouncycastle/asn1/DEROctetString",
                                                     asn1Encodable, "()[B", "getOctets");
            if (state){
                return result;
            }else{
                LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
                return nullptr;
            }
        }else{
            ThrowException(env, "java/security/cert/CertificateParsingException",
                           "Expected DEROctetString");
        }

        return nullptr;
    }

    inline jobject GetAsn1EncodableFromBytes(JNIEnv *env, jbyteArray bytes){
        jobject objASN1InputStream = nullptr;
        jobject objASN1Encodable = nullptr;
        bool state = false;

        objASN1InputStream = invoke_func()->newInstance(env, "org/bouncycastle/asn1/ASN1InputStream", "([B)V");
        state = invoke_func()->callObjectMethod(env, &objASN1Encodable, "org/bouncycastle/asn1/ASN1InputStream",
                                        objASN1InputStream, "()Lorg/bouncycastle/asn1/ASN1Encodable;", "readObject");
        if (!state){
            ThrowException(env, "java/security/cert/CertificateParsingException",
                           "Failed to parse Encodable");
            return nullptr;
        }

        return objASN1Encodable;
    }

    inline jobject GetAsn1SequenceFromStream(JNIEnv *env, jobject asn1InputStream){
        jclass clsASN1OctetString = nullptr;
        jclass clsASN1Sequence = nullptr;
        jobject objASN1Primitive = nullptr;
        jbyteArray octets = nullptr;
        jobject objSeqInputStream = nullptr;
        bool state = false;
        std::string msg = "";

        state = invoke_func()->callObjectMethod(env, &objASN1Primitive, "org/bouncycastle/asn1/ASN1InputStream",
                                                asn1InputStream, "()Lorg/bouncycastle/asn1/ASN1Primitive;",
                                                "readObject");
        if (!state){
            LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
            return nullptr;
        }

        clsASN1OctetString = env->FindClass("org/bouncycastle/asn1/ASN1OctetString");
        if (!env->IsInstanceOf(objASN1Primitive, clsASN1OctetString)) {
            msg = "Expected octet stream, found ";
            ThrowExceptionDetail(env, objASN1Primitive, "java/security/cert/CertificateParsingException",
                                 msg, "");
            return nullptr;
        }

        state = invoke_func()->callObjectMethod(env, (jobject *)&octets, "org/bouncycastle/asn1/ASN1OctetString",
                                                objASN1Primitive, "()[B", "getOctets");
        if (!state){
            LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
            return nullptr;
        }

        objSeqInputStream = invoke_func()->newInstance(env, "org/bouncycastle/asn1/ASN1InputStream", "([B)V", octets);
        if(nullptr == objSeqInputStream){
            LOGE("[-] %s %d invoke_func()->newInstance faild ", __FUNCTION__ , __LINE__);
            return nullptr;
        }

        state = invoke_func()->callObjectMethod(env, &objASN1Primitive, "org/bouncycastle/asn1/ASN1InputStream",
                                        objSeqInputStream, "()Lorg/bouncycastle/asn1/ASN1Primitive;", "readObject");
        if (!state){
            LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
            return nullptr;
        }

        clsASN1Sequence = env->FindClass("org/bouncycastle/asn1/ASN1Sequence");
        if (!env->IsInstanceOf(objASN1Primitive, clsASN1Sequence)) {
            msg = "Expected sequence, found ";
            ThrowExceptionDetail(env, objASN1Primitive, "java/security/cert/CertificateParsingException",
                                 msg, "");
            return nullptr;
        }

        return objASN1Primitive;
    }

    inline jobject GetAsn1SequenceFromBytes(JNIEnv *env, jbyteArray bytes){
        jobject objASN1InputStream = nullptr;

        objASN1InputStream = invoke_func()->newInstance(env, "org/bouncycastle/asn1/ASN1InputStream",
                                                        "([B)V", bytes);
        if (nullptr != objASN1InputStream){
            return GetAsn1SequenceFromStream(env, objASN1InputStream);
        }else{
            ThrowException(env, "java/security/cert/CertificateParsingException", "Failed to parse SEQUENCE");
            return nullptr;
        }
    }

    inline std::set<int> GetIntegersFromAsn1Set(JNIEnv* env, jobject set){
        jclass clsASN1Set = nullptr;
        jobject objEnumeration = nullptr;
        jobject objASN1Integer = nullptr;
        jboolean result = JNI_FALSE;
        bool state = false;
        std::string msg = "";
        std::set<int> resultSet;

        clsASN1Set = env->FindClass("org/bouncycastle/asn1/ASN1Set");
        if (!env->IsInstanceOf(set,clsASN1Set)){
            msg = "Expected sequence, found ";
            ThrowExceptionDetail(env, set, "java/security/cert/CertificateParsingException",
                                 msg, "");
            return {};
        }

        state = invoke_func()->callObjectMethod(env, &objEnumeration, "org/bouncycastle/asn1/ASN1Set",
                                                set, "()Ljava/util/Enumeration;", "getObjects");
        if (!state){
            LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
            return {};
        }

        state = invoke_func()->callBooleanMethod(env, &result, "java/util/Enumeration", objEnumeration,
                                                 "()Z", "hasMoreElements");
        if (!state){
            LOGE("[-] %s %d invoke_func()->callBooleanMethod faild ", __FUNCTION__ , __LINE__);
            return {};
        }

        while(result){
            state  = invoke_func()->callObjectMethod(env, &objASN1Integer, "java/util/Enumeration", objEnumeration,
                                            "()Ljava/lang/Object;", "nextElement");
            if (!state){
                LOGE("[-] %s %d invoke_func()->callObjectMethod faild ", __FUNCTION__ , __LINE__);
                return {};
            }else{
                resultSet.insert(GetIntegerFromAsn1(env, objASN1Integer));
            }

            state = invoke_func()->callBooleanMethod(env, &result, "java/util/Enumeration", objEnumeration,
                                                     "()Z", "hasMoreElements");
            if (!state){
                LOGE("[-] %s %d invoke_func()->callBooleanMethod faild ", __FUNCTION__ , __LINE__);
                return {};
            }
        }

        return resultSet;
    }

    inline jboolean GetBooleanFromAsn1(JNIEnv *env, jobject asn1Encodable){
        jclass clsASN1Boolean = nullptr;
        jobject objTrue = nullptr;
        jobject objFalse = nullptr;
        jboolean result = JNI_FALSE;
        bool state = false;
        std::string msg = "";

        clsASN1Boolean = env->FindClass("org/bouncycastle/asn1/ASN1Boolean");
        if (!env->IsInstanceOf(asn1Encodable, clsASN1Boolean)) {
            msg = "Expected sequence, found ";
            ThrowExceptionDetail(env, asn1Encodable, "java/security/cert/CertificateParsingException",
                                 msg, "");
            return JNI_FALSE;
        }

        state = invoke_func()->getStaticObject(env, &objTrue, "org/bouncycastle/asn1/ASN1Boolean", "TRUE",
                                       "Lorg/bouncycastle/asn1/ASN1Boolean;");
        if (!state){
            LOGE("[-] %s %d invoke_func()->getStaticObject faild ", __FUNCTION__ , __LINE__);
            return JNI_FALSE;
        }
        state = invoke_func()->getStaticObject(env, &objTrue, "org/bouncycastle/asn1/ASN1Boolean", "FALSE",
                                               "Lorg/bouncycastle/asn1/ASN1Boolean;");
        if (!state){
            LOGE("[-] %s %d invoke_func()->getStaticObject faild ", __FUNCTION__ , __LINE__);
            return JNI_FALSE;
        }

        if(invoke_func()->callBooleanMethod(env, &result, "org/bouncycastle/asn1/ASN1Boolean",
                                            asn1Encodable, "(Ljava/lang/Object;)Z", "equals", objTrue)){
            return JNI_TRUE;
        }else if(invoke_func()->callBooleanMethod(env, &result, "org/bouncycastle/asn1/ASN1Boolean",
                                                  asn1Encodable, "(Ljava/lang/Object;)Z", "equals", objFalse)){
            return JNI_FALSE;
        }else{
            ThrowException(env, "java/security/cert/CertificateParsingException",
                           "DER-encoded boolean values must contain either 0x00 or 0xFF");
            return JNI_FALSE;
        }
    }
};

#endif // NATIVE_ASN1UTILS_HPP