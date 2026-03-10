@file:OptIn(ExperimentalSerializationApi::class)
package com.xiaxi.safety.datastore.model

import kotlinx.serialization.ExperimentalSerializationApi
import kotlinx.serialization.Serializable
import kotlinx.serialization.decodeFromByteArray
import kotlinx.serialization.encodeToByteArray
import kotlinx.serialization.protobuf.ProtoBuf
import java.io.InputStream
import java.io.OutputStream

/**
 * 应用设置数据模型
 * 这是应用程序配置系统的核心数据类，负责管理和持久化用户的所有偏好设置
 * 使用 Protocol Buffers 进行序列化，提供高效的存储和读取性能
 */
@Serializable
data class AppSettings(
    /**
     * App 界面语言设置，默认值为系统语言
     */
    val language: Language = Language.SYSTEM_DEFAULT,

    /**
     * 主题颜色设置
     * @default 鼠尾草绿色 - 自然的灰绿色
     */
    val themeColor: ThemeColor = ThemeColor.WarmTaupe,

) {
    /**
     * 将设置对象序列化并写入输出流
     * 使用 Protocol Buffers 将当前的设置对象转换为二进制格式
     * 然后写入到指定的输出流中。这个方法主要用于数据持久化，与 DataStore 配合使用来保存用户设置
     *
     * @param output 目标输出流，通常是文件流或内存流
     * @throws Exception 当序列化失败或写入流失败时抛出异常
     */
    fun encodeTo(output: OutputStream) = output.write(
        ProtoBuf.encodeToByteArray(this)
    )

    /** Kotlin 没有 static，所以想要类似的效果，就要用 companion object */
    companion object {
        /**
         * 从输入流反序列化设置对象
         * 从指定的输入流中读取二进制数据，并使用 Protocol Buffers
         * 将其反序列化为 AppSettings 对象，用于从持久化存储中恢复用户设置
         *
         * @param input 包含序列化数据的输入流
         * @return 反序列化后的 AppSettings 对象
         * @throws Exception 当反序列化失败时抛出异常
         */
        fun decodeFrom(input: InputStream): AppSettings =
            ProtoBuf.decodeFromByteArray(input.readBytes())
    }
}