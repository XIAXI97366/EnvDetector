package com.xiaxi.safety.repository

import com.xiaxi.safety.datastore.AppSettingsDataStore
import com.xiaxi.safety.datastore.model.Language
import javax.inject.Inject

/**
 * App 设置仓储类
 * 这是应用设置模块的仓储层实现，遵循 Repository 设计模式，为上层组件（ViewModel、UI）提供统一的数据访问接口
 *
 * Repository 模式的核心价值：
 * - 数据源抽象：隐藏数据存储的具体实现（DataStore、数据库、网络等）
 * - 业务逻辑集中：将与数据相关的业务逻辑集中在仓储层
 * - 测试友好：便于进行单元测试和集成测试
 * - 缓存管理：可以在此层实现数据缓存和同步策略
 * - 多数据源协调：可以协调多个数据源的数据一致性
 *
 * 在本应用架构中的位置：
 * ```
 * UI Layer (Compose)
 *        ↓
 * ViewModel Layer
 *        ↓
 * Repository Layer  ← 当前类
 *        ↓
 * DataStore Layer (AppSettingsDataStore)
 *        ↓
 * File System (Protocol Buffers)
 */
class AppSettingsRepository @Inject constructor(
    private val userPreferencesDataSource: AppSettingsDataStore,
) {

    /**
     * App 设置数据流
     */
    val data get() = userPreferencesDataSource.data

    /**
     * 设置 App 的界面语言
     */
    suspend fun setLanguage(value: Language) = userPreferencesDataSource.setLanguage(value)
}