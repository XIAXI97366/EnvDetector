```
# xiaxi97366.jks 所有的信息都是 973668008 alias还是 key0
# 导出 jks 中的 crt 证书 keytool -export -alias key0 -file xiaxi97366.crt -keystore xiaxi97366.jks
```

# KeyAttestation

```
# 暂时不支持三星设备、StrongBox、uniqueIdIncluded (位于 KeyAttestation.java)

# 需要注意的是若存在 EAT_OID 则一定不存在 ASN1_OID，原因如下
		在一张证明证书里，EAT 扩展和 ASN.1 扩展是两种"等价的证明载体/编码"
    二者都承载"同一类语义"的证明信息（挑战值、KeyMint/Keymaster 版本、安全级别、SW/TEE 授权列表、RootOfTrust 等）
    但编码格式各不同：EAT 是 CBOR，ASN.1 是 DER
    同时存在会带来解析优先级、字段冲突、大小开销及一致性验证等问题
    所以为避免误用与安全歧义，"发现 EAT 就不允许 ASN.1 存在"，否则直接抛错
    备注一：但需要注意的是当前设备的扩展部分数据如何解析是根据 OID 决定的，而不是 attestationVersion 所决定
        OID == 1.3.6.1.4.1.11129.2.1.25 → 走 EAT 解析
        OID == 1.3.6.1.4.1.11129.2.1.17 → 走 ASN.1 解析
    备注二：存在 KeyMint 设备仍给出 ASN.1 扩展的情况（兼容/过渡实现或厂商实现）
        即 attestationVersion = 200（KeyMint 2.0 语义），而证书只有 ASN.1 OID，
        就会走 Asn1Attestation 解析，当然这只是格式仍用 ASN.1，但字段语义已按 KeyMint 解释，反之亦然

# tbsCertificate 是 X.509 证书里“待签名的证书主体”（to-be-signed Certificate），它包含证书的全部语义性内容，随后由发行者（父证书/CA）的私钥对它进行签名生成完整证书
    TBSCertificate ::= SEQUENCE {
        version,
        serialNumber,
        signature,
        issuer,                  -- 发行者
        validity,                -- 有效期
        subject,                 -- 持有者
        subjectPublicKeyInfo,    -- 持有者公钥
        extensions               -- 扩展（如 KeyUsage、Attestation 扩展等）
    }

# 如果单纯只是为了获取扩展部分数据中的设备相关信息 和 RootOfTrust 根本就没有必要创建新的证书（可以用现有证书就行 Alias == reveny 和 reveny_persistent）

# 区分认证证书（Alias也就是叶子证书）和认证密钥证书（AttestKeyAlias）的方法（按优先级）
    查看是否包含“认证扩展”并检查用途
        解析扩展成功，TEE/hardwareEnforced 的 purposes 包含 KM_PURPOSE_ATTEST_KEY → 认证密钥证书（Attesting Key/PAK/RKP 中间证书）
        解析扩展成功，purposes 不含 KM_PURPOSE_ATTEST_KEY → 认证证书（业务密钥的叶子证书）
        解析扩展失败但含 ProvisioningInfo（OID 1.3.6.1.4.1.11129.2.1.30）→ 多见于 RKP 的认证密钥证书
    位置与角色
        叶子证书：认证证书（承载 attestation 扩展与设备/RootOfTrust 等）
        上一级中间证书：认证密钥证书（签发叶子，不承载 attestation 扩展，RKP 时常含 ProvisioningInfo）

# 获取扩展部分的数据根本不需要认证密钥证书只需要认证证书（即叶子证书也就是通过 Alias 生成的证书）
```

# 动态检测类

```
antiDebug（双子看护
    TracerPid(parent) = 子进程1（child1）的 pid
    TracerPid(child1) = 子进程2（child2）的 pid
    TracerPid(child2) = 0）{
    guardPorcess{
    	// 检测目标进程的status文件记录的tracePid是否合规
      bool static check_process_stopped(pid_t pid);
      // 检测目标进程包含的所有线程中的status文件记录的tracePid是否合规
      bool static scan_process_threads(pid_t pid);
    }
    // 开启双子看护，并创建用于检测线程和监视线程
    static int start_guards();
    // 检测线程的顺序为（child1 -> parent，child2 -> child1，parent -> child2）
    static void *check_loop(void *arg);
    // 监视线程（child1 <-> parent，child2 <-> parent）
    static void *monitor_child_loop(void *arg);
    static void *monitor_parent_loop(void *arg);	
}

antiFrida{
    static int check_maps();	// 检查maps中是否带有frida特征
    static int detect_frida_threads();	// 读取线程中是否带有frida的特征，仅限于官方最新版本16.3.3有效
    static int check_frida_by_port(int port);	// 检查frida端口是否正在运行
    // 读取/proc/net/tcp6和/proc/net/tcp中是否存在frida端口，适用于安卓10以下
    static int check_frida_by_port2(const char *port, pid_t pid); 	
}
   
antiXposed{
	static bool check_maps();	// 检查maps中是否带有xposed特征
	static bool check_stack(JNIEnv* env);	// 检查java端堆栈中是否存在xposed特征
	static bool check_class(JNIEnv* env);	// 检查ClassLoader中是否存在xposed特征的类
	static bool check_service(JNIEnv* env);	// 检查是否启用带有xposed特征的服务
	static bool check_process();	// 检查/system/bin/app_process的内存范围中是否xpoesd特征
}

antiPlt{
	int check_rel_plt(elf_dyn_info *self);	// 检查.rel.plt或.rela.plt表项中的函数地址是否被修改
	int check_rel_dyn(elf_dyn_info *self);	// 检查.rel.dyn或.rela.dyn表项中的全局变量所记录的地址是否被修改
	int check_rel_android(elf_dyn_info *self);	// 未实现
}

checkMaps{
	bool check_maps_valid();	// 通过fd反查是否伪造了maps文件
	bool is_zygote_injected(); // 根据jit内存段的属性判断当前进程是否被zygote模块注入（该方案仅针对 Shamiko 前期方案）
	// 该项用于扫描maps中所有可执行内存，如果路径既不是以"/"开头，也不是"[vdso]"，或者路径以"/dev/zero"开头，则认为存在注入
	// 剩余的项如果maps中的inode和stat对应路径的inode不一致，或常规路径检查发现端倪，则认为存在注入
	bool is_map_segment_compliance();
}
	

	

```

# 环境检测类

```
class romEnv{
	static bool check_bl_enabled1();						// 通过获取ro.boot.vbmeta.device_state属性检测BL是否解锁
  static bool check_bl_enabled2(JNIEnv *env, jobject keyAttestaion);	// 通过rootOfTrust检测BL是否解锁
    
  static bool check_inline_code_flag();				// 检测是否安装类XPosed框架（但在KernelSU的LSPosed中失效）
  static bool check_danger_app(); 						// 检测当前设比环境中是否安装危险应用
  static bool checkr_rom_customize(); 				// 对当前设备进行完整性校验
  static bool check_usb_debug();							// 通过获取属性检测ADB是否开启  
  static bool check_rom_userdebug();          // 通过设备指纹检测ROM是否为USERDEBUG
};
```

# 设备信息类

    class romEnv{
    	static std::string getDeviceBrand();        // 获取当前ROM的品牌名
    	static std::string getDeviceDevice();       // 获取当前ROM的设备名
    	static std::string getDeviceManufacturer(); // 获取当前ROM的制造商名
    	static std::string getDeviceModel();        // 获取当前ROM的机型名
    	static std::string getDeviceProduct();      // 获取当前ROM的产品名
    }
    

# 错误处理类











## 未处理

#### 2、wchan 检测读取 /proc/self/wchan，检查线程当前在内核中的等待状态。如果检测到线程因调试而阻塞，就会触发异常处理

![image-20250912161910459](/Users/xiaxi/Library/Application Support/typora-user-images/image-20250912161910459.png)
