//
// Created by 97366 on 2024/6/11.
//

#include "helper.h"

void get_rom_property(const char *property, char *buffer) {
    pfn_system_property_get sub_system_property_get = nullptr;
    void *handle = nullptr;

    handle = dlopen("libc.so",  RTLD_LAZY);
    if(NULL == handle){
        goto GET_ROM_PROPERTY_EXIT;
    }
    sub_system_property_get = (pfn_system_property_get)dlsym(handle, "__system_property_get");
    if (NULL == sub_system_property_get){
        goto GET_ROM_PROPERTY_EXIT;
    }
    sub_system_property_get(property, buffer);

GET_ROM_PROPERTY_EXIT:
    if (NULL != handle){
        dlclose(handle);
        handle = nullptr;
    }
    return;
}

int get_rom_sdk(){
    char buffer[64] = {0};
    get_rom_property("ro.build.version.sdk", buffer);
    if (0 == strlen(buffer)){
        return -1;
    }
    return atoi(buffer);
}



int read_line(int fd, char *ptr, unsigned int maxlen) {
    int n = 0;
    int rc = 0;
    char c = 0;

    for (n = 1; n < maxlen; n++) {
        if ((rc = sub_read(fd, &c, 1)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;
        }else if (rc == 0) {
            if (n == 1){
                return 0;    // EOF no data read
            }else{
                break;    // EOF, some data read
            }
        }else{
            return (-1);    // error
        }
    }
    *ptr = 0;
    return (n);
}

//设置函数地址所在内存页的权限为可读可执行
bool set_read(void *symbol) {
    uintptr_t address = (uintptr_t) symbol;
    uintptr_t page_size = (uintptr_t) getpagesize();
    uintptr_t base = address & ~(page_size - 1);
    // inline check read at most 20 bytes
    uintptr_t end = (address + 20 + page_size - 1) & -page_size;
#ifdef DEBUG
    LOGI("set r+x from %p to %p", base, end);
#endif
    //LOGD("setRead base->%llx end->%llx size->%llx", base, end, end - base);
    if (mprotect((void *) base, end - base, PROT_READ | PROT_EXEC)) {
#ifdef DEBUG
        LOGW("cannot mprotect: %s", strerror(errno));
#endif
        return false;
    } else {
        return true;
    }
}

//设置函数地址所在内存页的权限为可读可写可执行
bool set_write(void *symbol) {
    uintptr_t address = (uintptr_t) symbol;
    uintptr_t page_size = (uintptr_t) getpagesize();
    uintptr_t base = address & ~(page_size - 1);
    // inline check read at most 20 bytes
    uintptr_t end = (address + 20 + page_size - 1) & -page_size;
#ifdef DEBUG
    LOGI("set r+x from %p to %p", base, end);
#endif
    LOGD("setWrite base->%llx end->%llx size->%llx", base, end, end - base);
    if (mprotect((void *) base, end - base, PROT_READ | PROT_EXEC | PROT_WRITE)) {
#ifdef DEBUG
        LOGW("cannot mprotect: %s", strerror(errno));
#endif
        return false;
    }else {
        return true;
    }
}

//只要符合该函数的汇编检测规则就可以判定为目标函数处于hook状态
bool is_inline_hooked(void *symbol) {
    if (symbol == NULL) {
        return false;
    }
    set_read(symbol);
#if defined(__arm__) || defined(DEBUG_HOOK_IDE)

    // https://developer.arm.com/docs/ddi0597/b/base-instructions-alphabetic-order/ldr-literal-load-register-literal
// A1, !(P == 0 && W == 1), we don't check P and W
// cond 010P U0W1 1111 _Rt_ xxxx xxxx xxxx
#define IS_LDR_PC_A1(x) (((x) & 0xfe5ff000u) == 0xe41ff000u)
// T2
// 1111 1000 U101 1111 | _Rt_ xxxx xxxx xxxx
#define IS_LDR_PC_T2(x) (((x) & 0xf000ff7fu) == 0xf000f85fu)

// https://developer.arm.com/docs/ddi0597/b/base-instructions-alphabetic-order/b-branch
// A1
// cond 100 xxxx xxxx xxxx xxxx xxxx xxxx
#define IS_B_A1(x) (((x) & 0xff000000u) == 0xea000000u)
// T2
// 1110 0xxx xxxx xxxx
#define IS_B_T2(x) (((x) & 0xf800u) == 0xe000u)
// T4
// 1111 0Sxx xxxx xxxx | 10J1 Jxxx xxxx xxxx
//        -- imm10 --- |       --- imm11 ---
#define IS_B_T4(x) (((x) & 0xd000f800u) == 0x9000f000u)

// https://developer.arm.com/docs/ddi0597/b/base-instructions-alphabetic-order/nop-no-operation
// T1, hint should be 0000, we don't check
// 1011 1111 hint 0000
#define IS_NOP_T1(x) (((x) & 0xff0fu) == 0xbf00u)

// https://developer.arm.com/docs/ddi0597/b/base-instructions-alphabetic-order/mov-movs-register-move-register
// cydia use `mov r8, r8` for Nop
// T1, Mmmm is Rm, Dddd is Rd
// 0100 0110 DMmm mddd
#define _IS_MOV_T1(x) (((x) & 0xff00u) == 0x4600u)
#define _RM_MOV_T1(x) ((((x) & 0x78u) >> 3u))
#define _RD_MOV_T1(x) ((((x) & 0x80u) >> 4u) | ((x) & 7u))
#define IS_MOV_T1_RR(x) (_IS_MOV_T1(x) && _RM_MOV_T1(x) == _RD_MOV_T1(x))

// https://developer.arm.com/docs/ddi0597/b/base-instructions-alphabetic-order/bx-branch-and-exchange
// cydia use `bx`
// T1
// 0100	0111 0Rmm m000
#define IS_BX_T1(x) (((x) & 0xff87u) == 0x4700u)
#define RM_BX_T1(x) (((x) & 0x0078u) >> 3u)
#define IS_BX_PC_T1(x) ((x) == 0x4778u)

    uintptr_t address = (uintptr_t) symbol;
    if ((address & 1U) == 0) {
        uint32_t *value32 = (uint32_t *) address;
        if (IS_LDR_PC_A1(*value32)) {
#ifdef DEBUG_HOOK
            LOGW("(arm ldr pc) symbol: %p, value: %08x", symbol, *value32);
#endif
            return true;
        }
        if (IS_B_A1(*value32)) {
#ifdef DEBUG_HOOK
            LOGW("(arm b) symbol: %p, value: %08x", symbol, *value32);
#endif
            return true;
        }
#ifdef DEBUG
        LOGI("(arm) symbol: %p, value: %08x", symbol, *value32);
#endif
    } else {
        address = address & ~1U;
        uint16_t *value16 = (uint16_t *) address;
        uint32_t *value32 = (uint32_t *) address;
        if (IS_LDR_PC_T2(*value32)) {
#ifdef DEBUG_HOOK
            LOGW("(thumb ldr pc) symbol: %p, address: %p, value: %08x",
                 symbol, address, *value32);
#endif
            return true;
        }
        if (IS_B_T4(*value32)) {
#ifdef DEBUG_HOOK
            LOGW("(thumb b) symbol: %p, address: %p, value: %08x",
                 symbol, address, *value32);
#endif
            return true;
        }
        if (IS_B_T2(*value16)) {
#ifdef DEBUG_HOOK
            LOGW("(thumb b) symbol: %p, address: %p, value: %04x",
                 symbol, address, *value16);
#endif
            return true;
        }
        if (IS_NOP_T1(*value16) || IS_MOV_T1_RR(*value16)) {
#ifdef DEBUG_HOOK
            LOGW("(thumb nop) symbol: %p, address: %p, value: %04x",
                 symbol, address, *value16);
#endif
            address += 2;
            value16 = (uint16_t *) address;
            value32 = (uint32_t *) address;
        }
        if (IS_LDR_PC_T2(*value32)) {
#ifdef DEBUG_HOOK
            LOGW("(thumb ldr pc) symbol: %p, address: %p, value: %08x",
                 symbol, address, *value32);
#endif
            return true;
        }
        if (IS_BX_PC_T1(*value16) && IS_LDR_PC_A1(*(value32 + 1))) {
#ifdef DEBUG_HOOK
            LOGW("(thumb bx + arm ldr pc) symbol: %p, address: %p, value: %08x %08x",
                 symbol, address, *value32, *(value32 + 1));
#endif
            return true;
        }
#ifdef DEBUG
        LOGI("(thumb) symbol: %p, address: %p, value: %08x %08x",
             symbol, address, *value32, *(value32 + 1));
#endif
    }
#endif
#if defined(__aarch64__) || defined(DEBUG_HOOK_IDE)

    // https://developer.arm.com/docs/ddi0596/latest/base-instructions-alphabetic-order/b-branch
// 0001 01xx xxxx xxxx xxxx xxxx xxxx xxxx
//        ------------ imm26 -------------
#define IS_B(x) (((x) & 0xfc000000u) == 0x14000000u)

// https://developer.arm.com/docs/ddi0596/latest/base-instructions-alphabetic-order/ldr-literal-load-register-literal
// 0101 1000 xxxx xxxx xxxx xxxx xxxR tttt
//           -------- imm19 --------
#define IS_LDR_X(x) (((x) & 0xff000000u) == 0x58000000u)
#define X_LDR(x) ((x) & 0x1fu)

// https://developer.arm.com/docs/ddi0596/latest/base-instructions-alphabetic-order/adrp-form-pc-relative-address-to-4kb-page
// 1xx1 0000 xxxx xxxx xxxx xxxx xxxR dddd
//  lo       -------- immhi --------
#define IS_ADRP_X(x) (((x) & 0x9f000000u) == 0x90000000u)
#define X_ADRP(x) ((x) & 0x1fu)

// https://developer.arm.com/docs/ddi0596/latest/base-instructions-alphabetic-order/br-branch-to-register
// 1101 0110 0001 1111 0000 00Rn nnn0 0000
#define IS_BR_X(x) (((x) & 0xfffffc0f) == 0xd61f0000u)
#define X_BR(x) (((x) & 0x3e0u) >> 0x5u)

// https://developer.arm.com/docs/ddi0596/latest/base-instructions-alphabetic-order/movz-move-wide-with-zero
// 1op1 0010 1hwx xxxx xxxx xxxx xxxR dddd
//              ------ imm16 -------
// for op, 00 -> MOVN, 10 -> MOVZ, 11 -> MOVK
#define IS_MOV_X(x) (((x) & 0x9f800000u) == 0x92800000u)
#define X_MOV(x) ((x) & 0x1fu)

    uint32_t *value32 = static_cast<uint32_t *>(symbol);
    if (IS_B(*value32)) {
#ifdef DEBUG_HOOK
        LOGW("(arm64 b) symbol: %p, value: %08x", symbol, *value32);
#endif
        return true;
    }
    if (IS_LDR_X(*value32) && IS_BR_X(*(value32 + 1))) {
        uint32_t x = X_LDR(*value32);
        if (x == X_BR(*(value32 + 1))) {
#ifdef DEBUG_HOOK
            LOGW("(arm64 ldr+br x%d) symbol: %p, value: %08x %08x",
                 x, symbol, *value32, *(value32 + 1));
#endif
            return true;
        }
    }
    if (IS_ADRP_X(*value32) && IS_BR_X(*(value32 + 1))) {
        uint32_t x = X_ADRP(*value32);
        if (x == X_BR(*(value32 + 1))) {
#ifdef DEBUG_HOOK
            LOGW("(arm64 adrp+br x%d) symbol: %p, value: %08x %08x",
                 x, symbol, *value32, *(value32 + 1));
#endif
            return true;
        }
    }
    if (IS_MOV_X(*value32)) {
        uint32_t x = X_MOV(*value32);
        for (int i = 1; i <= 4; ++i) {
            if (IS_BR_X(*(value32 + i))) {
                if (x != X_BR(*(value32 + i))) {
                    break;
                }
#ifdef DEBUG_HOOK
                for (int k = 0; k < i; ++k) {
                    LOGW("(arm64 mov x%d) symbol: %p, value: %08x",
                         x, symbol + sizeof(uint32_t) * k, *(value32 + k));
                }
                LOGW("(arm64  br x%d) symbol: %p, value: %08x",
                     x, symbol + sizeof(uint32_t) * i, *(value32 + i));
#endif
                return true;
            } else if (IS_MOV_X(*(value32 + i))) {
                if (x != X_MOV(*(value32 + i))) {
                    break;
                }
            }
        }
    }
#ifdef DEBUG
    LOGI("(arm64) symbol: %p, value: %08x %08x", symbol, *value32, *(value32 + 1));
#endif
#endif
    return false;
}

jobject get_application(JNIEnv *env){

}

const char *get_app_native_lib(JNIEnv *env){
    jobject objApplication = nullptr;
    invoke_func()->getStaticObject(env, &objApplication, "com/xx/shell/ACF", "application", "Landroid/app/Application;");
    jclass clsContext = env->FindClass("android/content/Context");
    jmethodID mtdGetApplicationInfo = env->GetMethodID(clsContext,
                                                       "getApplicationInfo",
                                                       "()Landroid/content/pm/ApplicationInfo;");

    jobject objApplicationInfo = env->CallObjectMethod(objApplication, mtdGetApplicationInfo);
    jclass clsApplicationInfo = env->FindClass("android/content/pm/ApplicationInfo");

    jfieldID fldNativeLibraryDir = env->GetFieldID(clsApplicationInfo,
                                                   "nativeLibraryDir", "Ljava/lang/String;");

    jobject objNativeLibraryDir = env->GetObjectField(objApplicationInfo, fldNativeLibraryDir);
    return const_cast<char *>(env->GetStringUTFChars((jstring)objNativeLibraryDir, JNI_FALSE));
}


const char *get_libart_path() {
    std::string art = {};
#if defined(__aarch64__)
    if (get_rom_sdk() >= ANDROID_R) {
        art = "/apex/com.android.art/lib64/libart.so";
    } else if (get_rom_sdk() >= ANDROID_Q) {
        art = "/apex/com.android.runtime/lib64/libart.so";
    } else {
        art = "/system/lib64/libart.so";
    }
#else
    if (get_rom_sdk() >= ANDROID_R) {
        art = "/apex/com.android.art/lib/libart.so";
    } else if (get_rom_sdk() >= ANDROID_Q) {
        art = "/apex/com.android.runtime/lib/libart.so";
    } else {
        art = "/system/lib/libart.so";
    }
#endif

    return art.c_str();
}


const char *get_linker_path() {
    std::string linker;
#if defined(__aarch64__)
    if (get_rom_sdk() >= ANDROID_R) {
        linker = "/apex/com.android.runtime/bin/linker64";
    } else if (get_rom_sdk() >= ANDROID_Q) {
        linker = "/apex/com.android.runtime/bin/linker64";
    } else {
        linker = "/system/bin/linker64";
    }
#else
    if (get_rom_sdk() >= ANDROID_R) {
        linker = "/apex/com.android.runtime/bin/linker";
    } else if (get_rom_sdk() >= ANDROID_Q) {
        linker = "/apex/com.android.runtime/bin/linker";
    } else {
        linker = "/system/bin/linker";
    }
#endif

    return linker.c_str();
}

const char *get_libc_plus_path() {
    std::string cplus;
#if defined(__aarch64__)
    cplus = "/system/lib64/libstdc++.so";
#else
    cplus = "/system/lib/libstdc++.so";

#endif
    return cplus.c_str();
}

const char *get_libc_path() {
    std::string libc = {};

#if defined(__aarch64__)
    if (get_rom_sdk() >= ANDROID_R) {
        libc = "/apex/com.android.runtime/lib64/bionic/libc.so";
    } else if (get_rom_sdk() >= ANDROID_Q) {
        libc = "/apex/com.android.runtime/lib64/bionic/libc.so";
    } else {
        libc = "/system/lib64/libc.so";
    }
#else
    if (get_rom_sdk() >= ANDROID_R) {
        libc = "/apex/com.android.runtime/lib/bionic/libc.so";
    } else if (get_rom_sdk() >= ANDROID_Q) {
        libc = "/apex/com.android.runtime/lib/bionic/libc.so";
    } else {
        libc = "/system/lib/libc.so";
    }
#endif
    return libc.c_str();
}

const char *get_libmediae_path() {
    std::string libmediae;

#if defined(__aarch64__)
    libmediae = "/system/lib64/libmediandk.so";
#else
    libmediae = "/system/lib/libmediandk.so";
#endif
    return libmediae.c_str();
}

const char *get_jit_path() {
    std::string libjit;

#if defined(__aarch64__)
    if (get_rom_sdk() >= ANDROID_R) {
        libjit = "/apex/com.android.art/lib64/libart-compiler.so";
    } else if (get_rom_sdk() >= ANDROID_Q) {
        libjit = "/apex/com.android.runtime/lib64/libart-compiler.so";
    } else {
        libjit = "/system/lib64/libart-compiler.so";
    }
#else
    if (get_rom_sdk() >= ANDROID_R) {
        libjit = "/apex/com.android.art/lib/libart-compiler.so";
    } else if (get_rom_sdk() >= ANDROID_Q) {
        libjit ="/apex/com.android.runtime/lib/libart-compiler.so";
    } else {
        libjit = "/system/lib/libart-compiler.so";
    }
#endif
    return libjit.c_str();
}

// 二次签名本质的目的都是为了通过系统的签名检测
bool check_certificate_2_V2(int fd, unsigned expected_size, const char *expected_sha256){
    struct stat apkInfo = {0};
    FILE *fp = nullptr;
    uint32_t EOCD = 0x06054b50;
    u_char byte = 0;
    uint32_t eocd = 0;
    uint32_t count = 0;
    uint32_t offset = 0;
    uint32_t central_dir_offset = 0;
    const char *APK_SIGNING_BLOCK_MAGIC = "APK Sig Block 42";
    uint32_t SIGNATURE_SCHEME_V2_MAGIC = 0x7109871A;
    char buf[16] = {0};
    uint64_t signing_blk_sz = 0;
    uint64_t u64_val = 0;
    uint32_t id = 0;
    uint32_t off = 0;
    bool v2_signing_valid = false;
    uint32_t v2_signing_blocks = 0;
    bool v3_signing_exist = false;
    bool v3_1_signing_exist = false;
    loff_t pos = 0;
    bool ret = false;

    if (fd <= 0){
        LOGE("[-] %s %d fd <= 0 ", __FUNCTION__ , __LINE__);
        goto PROC_EXIT;
    }

    if(-1 == fstat(fd, &apkInfo)){
        LOGE("[-] %s %d fstat error ", __FUNCTION__ , __LINE__);
        goto PROC_EXIT;
    }

    // fdopen 返回的 fp 如果不关闭会导致程序异常
    fp = fdopen(fd, "r");
    if (nullptr == fp){
        LOGE("[-] %s %d fdopen error ", __FUNCTION__ , __LINE__);
        goto PROC_EXIT;
    }

    // 未找到 EOCD 也需要处理 fp
    // find EOCD flags >> 50 4b 05 06
    for (int i = 1; i <= apkInfo.st_size; ++i) {
        fseek(fp, apkInfo.st_size - i, SEEK_SET);
        count = fread(&byte, sizeof(byte), 1, fp);
        if(1 == count){
            if (0x06 == byte){
                offset = ftell(fp) - 4;
                fseek(fp, offset, SEEK_SET);
                fread(&eocd, sizeof(eocd), 1, fp);
                if (eocd == EOCD){
                    fseek(fp, offset + 0x10, SEEK_SET);
                    count = fread(&central_dir_offset, sizeof(central_dir_offset), 1, fp);
                    if (1 != count){
                        LOGE("[-] %s %d read central_dir_offset error ", __FUNCTION__ , __LINE__);
                        goto PROC_EXIT;
                    }
                    break;
                }
            }
        }else{
            LOGE("[-] %s %d fread count ==> %d", __FUNCTION__ , __LINE__, count);
            goto PROC_EXIT;
        }
    }

    // 定位到 central_dir_offset 偏移
    fseek(fp, central_dir_offset - 0x10, SEEK_SET);
    fread(buf, sizeof(buf), 1, fp);
    if(0 == sub_strncmp(buf, APK_SIGNING_BLOCK_MAGIC, sub_strlen(APK_SIGNING_BLOCK_MAGIC))){
        // 读取的 Sign Block Magic 的大小
        fseek(fp, central_dir_offset - 0x18, SEEK_SET);
        fread(&u64_val, sizeof(u64_val), 1, fp);
        fseek(fp, central_dir_offset - (u64_val + 0x8), SEEK_SET);
        fread(&signing_blk_sz, sizeof(signing_blk_sz), 1, fp);
        if (signing_blk_sz != u64_val){
            LOGE("[-] %s %d invalid signing block size", __FUNCTION__ , __LINE__);
            goto PROC_EXIT;
        }

        for (int i = 1; i <= 10; ++i) {
            fread(&u64_val, sizeof(u64_val), 1, fp);
            if (u64_val == signing_blk_sz){
                break;
            }
            fread(&id, sizeof(id), 1, fp);
            pos = ftell(fp);
            off = 4;
            if (id == SIGNATURE_SCHEME_V2_MAGIC){
                // LOGE("[+] %s %d v2_signing_exist ", __FUNCTION__ , __LINE__);
                v2_signing_blocks++;
                v2_signing_valid = check_V2_sign_block(fp, pos, expected_size, expected_sha256);
            }else if(id == 0xf05368c0){
                LOGE("[+] %s %d v3_signing_exist ", __FUNCTION__ , __LINE__);
                v3_signing_exist = true;
            }else if(id == 0x1b93ad61){
                v3_1_signing_exist = true;
                // LOGE("[+] %s %d v3_1_signing_exist ", __FUNCTION__ , __LINE__);
            }else{
                // LOGE("[-] %s %d Unknown id: 0x%08x" , __FUNCTION__ , __LINE__, id);
            }
            pos += (u64_val - off);
            fseek(fp, pos, SEEK_SET);
        }
    }else{
        LOGE("[-] %s %d APK is No Sign Block Magic", __FUNCTION__ , __LINE__);
    }

    if (!v2_signing_valid){
        LOGE("[-] %s %d SIGN V2 verify Not meeting expectations ", __FUNCTION__ , __LINE__);
    }else{
        ret = true;
    }

PROC_EXIT:
    if (nullptr != fp){
        fclose(fp);
    }
    if (0 != fd){
        sub_close(fd);
    }
    return ret;
}

// 可以计算壳的签名，但就算是获取了签名，壳如果使用了魔改的算法也无法定位用于比较的字符串（包括 strcmp 和 strncmp）
// 因为 strcmp 和 strncmp 功能都可以由加固方实现，所以定位签名校验处的希望不大
bool check_V2_sign_block(FILE *fp, u_long pos, unsigned expected_size, const char *expected_sha256){
    uint32_t signer_sequence_len = 0;
    uint32_t signer_len = 0;
    uint32_t signed_data_len = 0;
    uint32_t digests_sequence_len = 0;
    uint32_t certificates_len = 0;
    uint32_t certificate_len = 0;
    u_char cert[CERT_MAX_LENGTH] = {0};
    char sha256[MAX_LENGTH] = {0};

    fseek(fp, pos, SEEK_SET);
    fread(&signer_sequence_len, sizeof(signer_sequence_len), 1, fp);
    fread(&signer_len, sizeof(signer_len), 1, fp);
    fread(&signed_data_len, sizeof(signed_data_len), 1, fp);
    fread(&digests_sequence_len, sizeof(digests_sequence_len), 1, fp);
    pos = ftell(fp);

    //移动当前文件指针 + digests_sequence_len
    fseek(fp, pos + digests_sequence_len, SEEK_SET);
    fread(&certificates_len, sizeof(certificates_len), 1, fp);
    fread(&certificate_len, sizeof(certificate_len), 1, fp);

    if (expected_size != certificate_len){
        LOGE("[-] %s %d cert size Not meeting expectations expected_size -> %d certificate_len -> %d",
             __FUNCTION__ , __LINE__, expected_size, certificate_len);
        return false;
    }

    if (certificate_len > CERT_MAX_LENGTH){
        LOGE("[-] %s %d cert length overlimit ", __FUNCTION__ , __LINE__);
        return false;
    }

    // 此处获取的 cert 数据是.jks导出.crt证书后，其.crt证书的二进制
    // LOGE("[+] %s %d certificate_len = %d ", __FUNCTION__ , __LINE__, certificate_len);
    fread(cert, certificate_len, 1, fp);
    sub_strncpy(sha256, get_hash_2_SHA256(&cert[0], &cert[certificate_len]),
                SHA256_DIGEST_SIZE + 1);
    // LOGE("[+] %s %d certificate V2 SHA256 = %s ", __FUNCTION__ , __LINE__, sha256);
    if (0 != sub_strncmp(sha256, expected_sha256, SHA256_DIGEST_SIZE)){
        return false;
    }
    return true;
}

const char *get_hash_2_SHA256(u_char *blockBegin, u_char *blockEnd){
    picosha2::hash256_one_by_one hasher;

    hasher.process(blockBegin, blockEnd);
    hasher.finish();
    std::vector<unsigned char> hash(picosha2::k_digest_size);
    hasher.get_hash_bytes(hash.begin(), hash.end());
    std::string hex_str = picosha2::get_hash_hex_string(hasher);
    // LOGE("[+] %s %d hex_str.c_str %s ", __FUNCTION__ , __LINE__, hex_str.c_str());
    return hex_str.c_str();
}

// 通过 fd 反查是否伪造了 maps 文件
bool check_baseapk_valid(int fd, const char *filePath, ssize_t pathLen, int inode){
    char buf[PATH_MAX] = {0};
    std::string fdPath("/proc/");
    struct stat statBuf = {0};
    size_t len = 0;
    int dstFd = 0;
    char dstPath[MAX_LENGTH] = {0};
    char realPath[MAX_LENGTH] = {0};

    // 检查长度是否匹配
    fdPath.append(std::to_string(getpid())).append("/fd/").append(std::to_string(fd));
    len = sub_readlinkat(AT_FDCWD, fdPath.c_str(), buf, PATH_MAX);
    if (pathLen != len){
        LOGE("[-] %s %d file path length is not meeting expectations ", __FUNCTION__ , __LINE__);
        return true;
    }

    // 使用 open 打开 filePath 文件再次查看 fd 返回路径是否匹配
    dstFd = open(filePath, O_RDONLY);
    if (dstFd > 0){
        snprintf(dstPath, sizeof(dstPath), "/proc/self/fd/%d", dstFd);
        len = sub_readlinkat(AT_FDCWD, dstPath, realPath, MAX_LENGTH);
        // 检查长度是否匹配
        if (pathLen != len){
            LOGE("[-] %s %d file path length is not meeting expectations ", __FUNCTION__ , __LINE__);
            return true;
        }
    }else{
        LOGE("[-] %s %d open file is error ", __FUNCTION__ , __LINE__);
        return false;
    }

    // 检查路径是否匹配
    if (0 != sub_strncmp(realPath, buf, sub_strlen(realPath))){
        LOGE("[-] %s %d file path is not meeting expectations ", __FUNCTION__ , __LINE__);
        return true;
    }

    // 检查文件id、inode是否匹配
    if ( 0 > sub_fstat(dstFd, &statBuf)){
        LOGE("[-] %s %d call fstat is error ", __FUNCTION__ , __LINE__);
        return false;
    }else{
        if (inode != statBuf.st_ino){
            LOGE("[-] %s %d file inode is not meeting expectations ", __FUNCTION__ , __LINE__);
            return true;
        }
        if (1000 != statBuf.st_gid || 1000 != statBuf.st_uid){
            LOGE("[-] %s %d file gid or uid is not meeting expectations ", __FUNCTION__ , __LINE__);
            return true;
        }
    }

    LOGE("[+] %s %d base.apk path is meeting expectations ", __FUNCTION__ , __LINE__);
    return false;
}

void *inlineHooker(void *targetFunc, void *replaceFunc) {
    auto pageSize = sysconf(_SC_PAGE_SIZE);
    auto funcAddress = ((uintptr_t) targetFunc) & (-pageSize);
    mprotect((void *) funcAddress, pageSize, PROT_READ | PROT_WRITE | PROT_EXEC);

    void *originalFunc;
    if (HookUtils::Hooker((void *) targetFunc, (void *) replaceFunc, (void **) &originalFunc)) {
        return originalFunc;
    }
    return nullptr;
}

bool inlineUnHooker(void *originalFunc) {
    return HookUtils::unHook(originalFunc);
}