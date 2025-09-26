//
// Created by mrack on 2021/7/21.
//


#include "jniRefInvoke.h"

static bool get_field_object(JNIEnv *env, jobject *result,
                             const char *className, jobject obj, const char *filedName,
                             const char *filedSig);

static bool get_field_static_object(JNIEnv *env, jobject *result,
                                    const char *className, const char *filedName,
                                    const char *filedSig);

static bool get_field_static_int(JNIEnv *env, jint *result,
                                 const char *className, const char *filedName);

static bool get_field_int(JNIEnv *env, jint *result, const char *className,
                          jobject obj, const char *filedName);

static bool get_field_long(JNIEnv *env, jlong *result, const char *className,
                           jobject obj, const char *filedName);

static bool set_field_object(JNIEnv *env, const char *className,
                             const char *filedName, const char *filedSig, jobject jobj,
                             jobject jfiled_vaule);

static bool set_field_boolean(JNIEnv *env, const char *className,
                              const char *filedName, jobject jobj, jboolean jfiled_vaule);

static bool set_field_int(JNIEnv *env, const char *className,
                          const char *filedName, jobject jobj, jint jfiled_vaule);

static bool set_field_long(JNIEnv *env, const char *className,
                           const char *filedName, jobject jobj, jlong jfiled_vaule);

static bool set_field_static_ojbect(JNIEnv *env, const char *className,
                                    const char *filedName, const char *filedSig,
                                    jobject jfiled_vaule);

static bool invoke_static_int_method(JNIEnv *env, jint *result,
                                     const char *className, const char *methodSig,
                                     const char *methodName,
                                     ...);

static bool invoke_static_boolean_method(JNIEnv *env, jboolean *result,
                                         const char *className, const char *methodSig,
                                         const char *methodName,
                                         ...);

static bool invoke_static_void_method(JNIEnv *env, const char *className,
                                      const char *methodSig, const char *methodName, ...);

static bool invoke_static_object_method(JNIEnv *env, jobject *result,
                                        const char *className, const char *methodSig,
                                        const char *methodName,
                                        ...);

static bool invoke_object_method(JNIEnv *env, jobject *result,
                                 const char *className, jobject jobj, const char *methodSig,
                                 const char *methodName, ...);

static bool invoke_int_method(JNIEnv *env, jint *result, const char *className,
                              jobject jobj, const char *methodSig, const char *methodName, ...);

static bool invoke_long_method(JNIEnv *env, jlong *result, const char *className,
                        jobject jobj, const char *methodSig, const char *methodName, ...);

static bool invoke_boolean_method(JNIEnv *env, jboolean *result,
                                  const char *className, jobject jobj, const char *methodSig,
                                  const char *methodName, ...);

static bool invoke_void_method(JNIEnv *env, const char *className, jobject jobj,
                               const char *methodSig, const char *methodName, ...);

static jobject new_instance(JNIEnv *env, const char *className,
                            const char *methodSig, ...);

static jstring get_jstring_from_cstr(JNIEnv *env, const char *pStr);


static inline int exception_check_and_clear(JNIEnv *env) {
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }
    return 0;
}

bool get_field_object(JNIEnv *env, jobject *result, const char *className,
                      jobject obj, const char *filedName, const char *filedSig) {
    jclass clazz = env->FindClass(className);
    exception_check_and_clear(env);

    do {
        if (!clazz) {
            break;
        }

        jfieldID fieldId = env->GetFieldID(clazz, filedName, filedSig);
        exception_check_and_clear(env);
        if (!fieldId) {
            env->DeleteLocalRef(clazz);
            break;
        }

        *result = env->GetObjectField(obj, fieldId);

        exception_check_and_clear(env);
        env->DeleteLocalRef(clazz);
        return true;
    } while (0);
    return false;

}

bool get_field_static_object(JNIEnv *env, jobject *result,
                             const char *className, const char *filedName, const char *filedSig) {
    jclass clazz = env->FindClass(className);
    exception_check_and_clear(env);

    do {
        if (!clazz) {
            break;
        }
        jfieldID jfieldId_ = env->GetStaticFieldID(clazz, filedName, filedSig);
        exception_check_and_clear(env);

        if (!jfieldId_) {
            env->DeleteLocalRef(clazz);
            break;
        }
        *result = env->GetStaticObjectField(clazz, jfieldId_);

        exception_check_and_clear(env);
        env->DeleteLocalRef(clazz);
        return true;
    } while (0);
    return false;
}

bool get_field_static_int(JNIEnv *env, jint *result, const char *className,
                          const char *filedName) {
    jclass clazz = env->FindClass(className);
    exception_check_and_clear(env);

    do {
        if (!clazz) {
            break;
        }
        jfieldID jfieldId_ = env->GetStaticFieldID(clazz, filedName, "I");
        exception_check_and_clear(env);

        if (!jfieldId_) {
            env->DeleteLocalRef(clazz);
            break;
        }
        *result = env->GetStaticIntField(clazz, jfieldId_);

        exception_check_and_clear(env);
        env->DeleteLocalRef(clazz);
        return true;
    } while (0);
    return false;
}

bool get_field_int(JNIEnv *env, jint *result, const char *className,
                   jobject jobj, const char *filedName) {


    jclass clazz = env->FindClass(className);
    exception_check_and_clear(env);

    do {
        if (!clazz) {
            break;
        }
        jfieldID jfieldId_ = env->GetFieldID(clazz, filedName, "I");
        exception_check_and_clear(env);

        if (!jfieldId_) {
            env->DeleteLocalRef(clazz);
            break;
        }
        *result = env->GetIntField(jobj, jfieldId_);
        exception_check_and_clear(env);
        env->DeleteLocalRef(clazz);
        return true;
    } while (0);
    return false;
}

bool get_field_long(JNIEnv *env, jlong *result, const char *className,
                    jobject jobj, const char *filedName) {
    jclass clazz = env->FindClass(className);
    exception_check_and_clear(env);

    do {
        if (!clazz) {
            break;
        }
        jfieldID jfieldId_ = env->GetFieldID(clazz, filedName, "J");
        exception_check_and_clear(env);

        if (!jfieldId_) {
            env->DeleteLocalRef(clazz);
            break;
        }
        *result = env->GetLongField(jobj, jfieldId_);
        exception_check_and_clear(env);
        env->DeleteLocalRef(clazz);
        return true;
    } while (0);
    return false;
}

bool set_field_object(JNIEnv *env, const char *className, const char *filedName,
                      const char *filedSig, jobject jobj, jobject jfiled_vaule) {
    jclass clazz = env->FindClass(className);
    exception_check_and_clear(env);

    do {
        if (!clazz) {
            break;
        }

        jfieldID jfieldId_ = env->GetFieldID(clazz, filedName, filedSig);
        exception_check_and_clear(env);

        if (!jfieldId_) {
            env->DeleteLocalRef(clazz);
            break;
        }
        env->SetObjectField(jobj, jfieldId_, jfiled_vaule);
        exception_check_and_clear(env);
        env->DeleteLocalRef(clazz);
        return true;
    } while (0);
    return false;
}

bool set_field_boolean(JNIEnv *env, const char *className,
                       const char *filedName, jobject jobj, jboolean jfiled_vaule) {
    char filedSig[] = "Z";
    jclass clazz = env->FindClass(className);
    exception_check_and_clear(env);

    do {
        if (!clazz) {
            break;
        }
        jfieldID jfieldId_ = env->GetFieldID(clazz, filedName, filedSig);
        exception_check_and_clear(env);

        if (!jfieldId_) {
            env->DeleteLocalRef(clazz);
            break;
        }
        env->SetBooleanField(jobj, jfieldId_, jfiled_vaule);

        exception_check_and_clear(env);
        env->DeleteLocalRef(clazz);
        return true;
    } while (0);
    return false;
}

bool set_field_int(JNIEnv *env, const char *className, const char *filedName,
                   jobject jobj, jint jfiled_vaule) {
    char filedSig[] = "I";
    jclass clazz = env->FindClass(className);
    exception_check_and_clear(env);

    do {
        if (!clazz) {
            break;
        }

        jfieldID jfieldId_ = env->GetFieldID(clazz, filedName, filedSig);
        exception_check_and_clear(env);

        if (!jfieldId_) {
            env->DeleteLocalRef(clazz);
            break;
        }
        env->SetIntField(jobj, jfieldId_, jfiled_vaule);

        exception_check_and_clear(env);
        env->DeleteLocalRef(clazz);
        return true;
    } while (0);
    return false;
}

bool set_field_long(JNIEnv *env, const char *className, const char *filedName,
                    jobject jobj, jlong jfiled_vaule) {
    char filedSig[] = "J";
    jclass clazz = env->FindClass(className);
    exception_check_and_clear(env);

    do {
        if (!clazz) {
            break;
        }

        jfieldID jfieldId_ = env->GetFieldID(clazz, filedName, filedSig);
        exception_check_and_clear(env);

        if (!jfieldId_) {
            env->DeleteLocalRef(clazz);
            break;
        }
        env->SetLongField(jobj, jfieldId_, jfiled_vaule);

        exception_check_and_clear(env);
        env->DeleteLocalRef(clazz);
        return true;
    } while (0);
    return false;
}

bool set_field_static_ojbect(JNIEnv *env, const char *className,
                             const char *filedName, const char *filedSig, jobject jfiled_vaule) {
    return set_field_object(env, className, filedName, filedSig, NULL,
                            jfiled_vaule);
}

bool invoke_static_int_method(JNIEnv *env, jint *result, const char *className,
                              const char *methodSig, const char *methodName, ...) {
    jclass clazz = env->FindClass(className);
    exception_check_and_clear(env);
    do {
        if (!clazz) {
            break;
        }
        jmethodID jmethodId_ = env->GetStaticMethodID(clazz, methodName,
                                                      methodSig);
        exception_check_and_clear(env);

        if (!jmethodId_) {
            env->DeleteLocalRef(clazz);
            break;
        }
        va_list arg_ptr;

        va_start(arg_ptr, methodName);
        *result = env->CallStaticIntMethodV(clazz, jmethodId_, arg_ptr);

        exception_check_and_clear(env);
        va_end(arg_ptr);
        env->DeleteLocalRef(clazz);
        return true;
    } while (0);
    return false;
}

bool invoke_static_boolean_method(JNIEnv *env, jboolean *result,
                                  const char *className, const char *methodSig,
                                  const char *methodName,
                                  ...) {
    jclass clazz = env->FindClass(className);
    exception_check_and_clear(env);
    do {
        if (!clazz) {
            break;
        }
        jmethodID jmethodId_ = env->GetStaticMethodID(clazz, methodName,
                                                      methodSig);
        exception_check_and_clear(env);

        if (!jmethodId_) {
            env->DeleteLocalRef(clazz);
            break;
        }
        va_list arg_ptr;

        va_start(arg_ptr, methodName);
        *result = env->CallStaticBooleanMethodV(clazz, jmethodId_, arg_ptr);
        exception_check_and_clear(env);
        va_end(arg_ptr);
        env->DeleteLocalRef(clazz);
        return true;
    } while (0);
    return false;
}

bool invoke_static_void_method(JNIEnv *env, const char *className,
                               const char *methodSig, const char *methodName, ...) {
    jclass clazz = env->FindClass(className);
    exception_check_and_clear(env);
    do {
        if (!clazz) {
            break;
        }
        jmethodID jmethodId_ = env->GetStaticMethodID(clazz, methodName,
                                                      methodSig);
        exception_check_and_clear(env);

        if (!jmethodId_) {
            env->DeleteLocalRef(clazz);
            break;
        }
        va_list arg_ptr;
        va_start(arg_ptr, methodName);
        env->CallStaticVoidMethodV(clazz, jmethodId_, arg_ptr);

        exception_check_and_clear(env);
        va_end(arg_ptr);
        env->DeleteLocalRef(clazz);
        return true;
    } while (0);
    return false;
}

bool invoke_static_object_method(JNIEnv *env, jobject *result,
                                 const char *className, const char *methodSig,
                                 const char *methodName,
                                 ...) {
    jclass clazz = env->FindClass(className);
    exception_check_and_clear(env);
    do {
        if (!clazz) {
            break;
        }
        jmethodID jmethodId_ = env->GetStaticMethodID(clazz, methodName,
                                                      methodSig);
        exception_check_and_clear(env);

        if (!jmethodId_) {
            env->DeleteLocalRef(clazz);
            break;
        }
        va_list arg_ptr;

        va_start(arg_ptr, methodName);
        *result = env->CallStaticObjectMethodV(clazz, jmethodId_, arg_ptr);

        exception_check_and_clear(env);
        va_end(arg_ptr);
        env->DeleteLocalRef(clazz);
        return true;
    } while (0);
    return false;
}

bool invoke_object_method(JNIEnv *env, jobject *result, const char *className,
                          jobject jobj, const char *methodSig, const char *methodName, ...) {
    jclass clazz = env->FindClass(className);
    exception_check_and_clear(env);
    do {
        if (!clazz || !jobj) {
            break;
        }

        jmethodID jmethodId_ = env->GetMethodID(clazz, methodName, methodSig);
        exception_check_and_clear(env);
        if (!jmethodId_) {
            env->DeleteLocalRef(clazz);
            break;
        }
        va_list arg_ptr;
        va_start(arg_ptr, methodName);
        *result = env->CallObjectMethodV(jobj, jmethodId_, arg_ptr);

        exception_check_and_clear(env);
        va_end(arg_ptr);
        env->DeleteLocalRef(clazz);
        return true;
    } while (0);
    return false;
}

bool invoke_int_method(JNIEnv *env, jint *result, const char *className,
                       jobject jobj, const char *methodSig, const char *methodName, ...) {
    jclass clazz = env->FindClass(className);
    exception_check_and_clear(env);
    do {
        if (!clazz || !jobj) {
            break;
        }

        jmethodID jmethodId_ = env->GetMethodID(clazz, methodName, methodSig);
        exception_check_and_clear(env);
        if (!jmethodId_) {
            env->DeleteLocalRef(clazz);
            break;
        }
        va_list arg_ptr;
        va_start(arg_ptr, methodName);
        *result = env->CallIntMethodV(jobj, jmethodId_, arg_ptr);

        exception_check_and_clear(env);
        va_end(arg_ptr);
        env->DeleteLocalRef(clazz);
        return true;
    } while (0);
    return false;
}

bool invoke_long_method(JNIEnv *env, jlong *result, const char *className,
                        jobject jobj, const char *methodSig, const char *methodName, ...){
    jclass clazz = env->FindClass(className);
    exception_check_and_clear(env);
    do {
        if (!clazz || !jobj) {
            break;
        }

        jmethodID jmethodId_ = env->GetMethodID(clazz, methodName, methodSig);
        exception_check_and_clear(env);
        if (!jmethodId_) {
            env->DeleteLocalRef(clazz);
            break;
        }
        va_list arg_ptr;
        va_start(arg_ptr, methodName);
        *result = env->CallLongMethodV(jobj, jmethodId_, arg_ptr);

        exception_check_and_clear(env);
        va_end(arg_ptr);
        env->DeleteLocalRef(clazz);
        return true;
    } while (0);
    return false;
}

bool invoke_boolean_method(JNIEnv *env, jboolean *result, const char *className,
                           jobject jobj, const char *methodSig, const char *methodName, ...) {
    jclass clazz = env->FindClass(className);
    exception_check_and_clear(env);
    do {
        if (!clazz || !jobj) {
            break;
        }

        jmethodID jmethodId_ = env->GetMethodID(clazz, methodName, methodSig);
        exception_check_and_clear(env);
        if (!jmethodId_) {
            env->DeleteLocalRef(clazz);
            break;
        }
        va_list arg_ptr;
        va_start(arg_ptr, methodName);
        *result = env->CallBooleanMethodV(jobj, jmethodId_, arg_ptr);

        exception_check_and_clear(env);
        va_end(arg_ptr);
        env->DeleteLocalRef(clazz);
        return true;
    } while (0);
    return false;
}

bool invoke_void_method(JNIEnv *env, const char *className, jobject jobj,
                        const char *methodSig, const char *methodName, ...) {
    jclass clazz = env->FindClass(className);
    exception_check_and_clear(env);
    do {
        if (!clazz) {
            break;
        }
        jmethodID jmethodId_ = env->GetMethodID(clazz, methodName, methodSig);
        exception_check_and_clear(env);

        if (!jmethodId_) {
            env->DeleteLocalRef(clazz);
            break;
        }
        va_list arg_ptr;
        va_start(arg_ptr, methodName);
        env->CallVoidMethodV(jobj, jmethodId_, arg_ptr);

        exception_check_and_clear(env);
        env->DeleteLocalRef(clazz);
        va_end(arg_ptr);
        return true;
    } while (0);
    return false;
}


static bool invoke_novirtual_void_method(JNIEnv* env, const char* className, const char* methodName,
                                          const char* sig, jobject thiz, ...) {
    bool bRet = false;
    jclass clazz = env->FindClass(className);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        LOGE("[-] %s %d FindClass Error", __FUNCTION__, __LINE__);
        return bRet;
    }

    jmethodID methodId = env->GetMethodID(clazz, methodName, sig);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        LOGE("[-] %s %d GetMethodID Error", __FUNCTION__, __LINE__);
        return bRet;
    }

    va_list args;
    va_start(args, thiz);
    env->CallNonvirtualVoidMethodV(thiz, clazz, methodId, args);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        bRet = false;
    }else {
        bRet = true;
    }
    va_end(args);
    return bRet;
}

jobject new_instance(JNIEnv *env, const char *className, const char *methodSig,
                     ...) {
    const char *methodName = "<init>";
    jclass clazz = env->FindClass(className);
    exception_check_and_clear(env);

    if (!clazz) {
        return NULL;
    }
    jmethodID jmethodId_ = env->GetMethodID(clazz, methodName, methodSig);
    exception_check_and_clear(env);

    if (!jmethodId_) {
        env->DeleteLocalRef(clazz);
        return NULL;
    }
    va_list arg_ptr;
    va_start(arg_ptr, methodSig);
    jobject jobj = env->NewObjectV(clazz, jmethodId_, arg_ptr);
    env->CallVoidMethodV(jobj, jmethodId_, arg_ptr);

    exception_check_and_clear(env);
    va_end(arg_ptr);
    env->DeleteLocalRef(clazz);
    return jobj;
}

jstring get_jstring_from_cstr(JNIEnv *env, const char *pStr) {
    if (!pStr) {
        return NULL;
    }
    int strLen = strlen(pStr);
    jclass jstrObj = env->FindClass("java/lang/String");
    jmethodID methodId = env->GetMethodID(jstrObj, "<init>",
                                          "([BLjava/lang/String;)V");
    jbyteArray byteArray = env->NewByteArray(strLen);
    jstring encode = env->NewStringUTF("utf-8");

    env->SetByteArrayRegion(byteArray, 0, strLen, (jbyte *) pStr);

    jstring result = (jstring) env->NewObject(jstrObj, methodId, byteArray,
                                              encode);
    env->DeleteLocalRef(jstrObj);
    env->DeleteLocalRef(byteArray);
    env->DeleteLocalRef(encode);
    return result;
}

static char *get_cstr_from_jstring(JNIEnv* env, jstring jstr) {
    jobject result;

    jclass jclass_ = env->FindClass("java/lang/String");
    exception_check_and_clear(env);

    if (!jclass_) {
        return NULL;
    }
    jmethodID jmethodId_ = env->GetMethodID(jclass_, "getBytes", "()[B");
    exception_check_and_clear(env);

    if (!jmethodId_) {
        env->DeleteLocalRef(jclass_);
        return NULL;
    }

    result = env->CallObjectMethod(jstr, jmethodId_);
    exception_check_and_clear(env);
    env->DeleteLocalRef(jclass_);

    jbyteArray jdata = (jbyteArray) result;
    size_t len = env->GetArrayLength(jdata);
    jbyte* data = env->GetByteArrayElements(jdata, JNI_FALSE);
    const char *p = strdup(reinterpret_cast<const char *>(data));
    env->ReleaseByteArrayElements(jdata, data, len);
    return const_cast<char *>(p);
}

static INVOKE invokeMod = {get_field_object, get_field_static_object,
                               get_field_static_int, get_field_int,
                               get_field_long,set_field_object,
                               set_field_boolean, set_field_int, set_field_long,
                               set_field_static_ojbect, invoke_static_int_method,
                               invoke_static_boolean_method, invoke_static_void_method,
                               invoke_static_object_method, invoke_object_method,
                               invoke_int_method,invoke_long_method,invoke_boolean_method,
                               invoke_void_method, invoke_novirtual_void_method,
                               new_instance,get_jstring_from_cstr,get_cstr_from_jstring};

INVOKE *invoke_func() {
    return &invokeMod;
}