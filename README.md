# XIAXI


# xiaxi97366.jks 所有的信息都是 973668008 alias还是 key0
# 导出 jks 中的 crt 证书 keytool -export -alias key0 -file xiaxi97366.crt -keystore xiaxi97366.jks

# 暂时不支持三星设备、StrongBox、uniqueIdIncluded (位于 KeyAttestation.java)
# 需要注意的是若存在 EAT_OID，则必须不存在 ASN1_OID
# 同一张证明证书里，EAT 扩展和 ASN.1 扩展是两种“等价的证明载体/编码”，不能同时出现。代码强制这两者二选一，避免歧义与冲突，原因：
	二者都承载“同一类语义”的证明信息（挑战值、KeyMint/Keymaster 版本、安全级别、SW/TEE 授权列表、RootOfTrust 等）
    但编码格式各不同：EAT 是 CBOR，ASN.1 是 DER
	同时存在会带来解析优先级、字段冲突、大小开销及一致性验证等问题
    规范与业界实践通常要求按设备实现选择其一（KeyMint→EAT，Keymaster→ASN.1）
	所以为避免误用与安全歧义，此实现选择“发现 EAT 就不允许 ASN.1 共存”，否则直接抛错
