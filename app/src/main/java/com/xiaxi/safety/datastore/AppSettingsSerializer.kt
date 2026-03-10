package com.xiaxi.safety.datastore

import androidx.datastore.core.CorruptionException
import androidx.datastore.core.Serializer
import com.xiaxi.safety.datastore.model.AppSettings
import kotlinx.serialization.SerializationException
import java.io.InputStream
import java.io.OutputStream
import javax.inject.Inject

/**
 * App 设置序列化器
 * 这是 DataStore 与 AppSettings 数据模型之间的桥梁，负责处理应用设置的序列化和反序列化操作
 * 将 AppSettings 对象序列化为二进制数据并保存到磁盘
 * 从磁盘读取二进制数据并反序列化为 AppSettings 对象
 */

/**
 * 该 AppSettingsSerializer 类构造添加的 @Inject 注解，在需要创建 AppSettingsSerializer 对象时
 * 会由 Dagger 生成的 AppSettingsSerializer_Factory 工厂类，Hilt 在运行时用它创建实例
 */
class AppSettingsSerializer @Inject constructor() : Serializer<AppSettings> {
    /**
     * 默认应用设置值
     * 当应用首次启动或数据文件不存在时，DataStore 会使用这个默认值来初始化应用设置
     * @see AppSettings AppSettings 构造函数中的默认参数，Language.SYSTEM_DEFAULT 和 ThemeColor.CoralBurst
     */
    override val defaultValue = AppSettings()

    /**
     * 从输入流读取并反序列化应用设置
     * 这个方法从 DataStore 的持久化存储中读取二进制数据
     * 并将其反序列化为 AppSettings 对象，整个过程是异步的，不会阻塞调用线程
     *
     * @param input 包含序列化数据的输入流，通常来自 DataStore 的文件系统
     * @return 反序列化后的 AppSettings 对象，包含所有用户配置
     * @throws CorruptionException 当数据损坏或反序列化失败时抛出
     *
     * @see AppSettings.decodeFrom 实际的反序列化实现
     * @see CorruptionException DataStore 的数据损坏异常
     */
    override suspend fun readFrom(input: InputStream) =
        try {
            AppSettings.Companion.decodeFrom(input)
        } catch (e: SerializationException) {
            // 将序列化异常转换为 DataStore 可识别的数据损坏异常
            // 这样 DataStore 就知道需要使用默认值或进行数据恢复
            throw CorruptionException("Failed to read proto", e)
        }

    /**
     * 将应用设置序列化并写入输出流
     * 这个方法将 AppSettings 对象序列化为二进制格式
     * 并写入到 DataStore 的持久化存储中，整个过程是异步的，确保不会影响应用的响应性能
     *
     * @param t 要序列化的 AppSettings 对象，包含所有用户配置
     * @param output 目标输出流，DataStore 会将其映射到具体的存储位置
     * @throws Exception 当序列化或写入失败时抛出相应异常
     *
     * @see AppSettings.encodeTo 实际的序列化实现
     * @see androidx.datastore.core.DataStore.updateData DataStore更新方法
     */
    override suspend fun writeTo(t: AppSettings, output: OutputStream) {
        // 委托给 AppSettings 对象自身的序列化方法
        // 这样保持了序列化逻辑的封装性和一致性
        t.encodeTo(output)
    }
}