package com.xiaxi.safety.datastore

import androidx.datastore.core.DataStore
import com.xiaxi.safety.datastore.model.*
import kotlinx.coroutines.flow.Flow
import javax.inject.Inject

/**
 * App 设置数据存储访问层
 * 这是 App 设置持久化存储的高级封装类，提供了类型安全、响应式的数据访问接口
 * 它在 DataStore 的基础上构建了一层业务抽象，使得上层组件（如 Repository 和 ViewModel）
 * 可以更方便地进行应用设置的读取和修改操作
 */
class AppSettingsDataStore @Inject constructor(
    private val appSettings: DataStore<AppSettings>
) {
    /**
     * App 设置数据流
     * 这是一个响应式的数据流，任何对 App 设置的修改都会通过这个 Flow
     * 自动推送给所有的观察者，UI 层可以实时响应设置的变化，无需手动刷新或轮询检查
     * data 的类型是 Flow<AppSettings>，表示会持续发出 AppSettings 的数据流
     * 流里每次发出的元素类型才是 AppSettings
     */
    val data: Flow<AppSettings> = appSettings.data

    /**
     * 私有的通用更新方法，为所有具体的设置更新操作提供统一的实现基础
     * 它接收一个 lambda 表达式，该表达式描述了如何从当前设置创建新的设置对象
     */
    private suspend fun update(prefs: AppSettings.() -> AppSettings) {
        appSettings.updateData { it.prefs() }
    }

    /**
     * 设置 App 界面语言
     */
    suspend fun setLanguage(value: Language) = update { copy(language = value) }
}