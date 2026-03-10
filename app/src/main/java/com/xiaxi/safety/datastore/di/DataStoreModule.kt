package com.xiaxi.safety.datastore.di

import android.content.Context
import androidx.datastore.core.DataStore
import androidx.datastore.core.DataStoreFactory
import androidx.datastore.dataStoreFile
import com.xiaxi.safety.datastore.AppSettingsSerializer
import com.xiaxi.safety.datastore.model.AppSettings
import dagger.Module
import dagger.Provides
import dagger.hilt.InstallIn
import dagger.hilt.android.qualifiers.ApplicationContext
import dagger.hilt.components.SingletonComponent
import javax.inject.Singleton

/**
 * Hilt 依赖框架注入模块 -> DataStore 配置
 * 该模块负责提供 DataStore 相关的依赖，用于 App 设置的持久化存储
 */
@Module  // 标记这是一个 Hilt 模块，包含依赖提供方法
@InstallIn(SingletonComponent::class)  // 将此模块安装到 SingletonComponent 中，确保单例生命周期
object DataStoreModule {
    /**
     * 提供用户偏好设置的 DataStore 实例
     * @param context 应用上下文，通过 @ApplicationContext 注解注入
     * @param appSettingsSerializer App 设置序列化器，用于数据的序列化和反序列化
     * @return DataStore<AppSettings> 用于存储 App 设置的 DataStore 实例
     */
    @Provides  // 标记这是一个依赖提供方法
    @Singleton  // 确保在整个应用生命周期中只创建一个实例
    fun providesUserPreferencesDataStore(
        @ApplicationContext context: Context,               // 注入应用级别的Context
        appSettingsSerializer: AppSettingsSerializer        // 注入序列化器依赖
    ): DataStore<AppSettings> = DataStoreFactory.create(
        serializer = appSettingsSerializer
    ){
        // 创建数据存储文件，文件名为"app_settings.pb"，位于 /data/data/com.xiaxi.safety/files/datastore/
        context.dataStoreFile("app_settings.pb")
    }

    /**
     * Hilt 依赖注入在这里的工作流程：
     *
     * 1. 编译时：
     *    - Hilt 扫描到 @Module 注解，识别这是一个依赖提供模块
     *    - @InstallIn(SingletonComponent::class) 告诉 Hilt 将此模块安装到应用级组件
     *    - 生成相应的依赖注入代码
     *
     * 2. 运行时：
     *    - 当有类需要 DataStore<AppSettings> 依赖时（如 AppSettingsDataStore）
     *    - Hilt 会调用 providesUserPreferencesDataStore 方法
     *    - 自动注入所需的 Context 和 AppSettingsSerializer 参数
     *    - 返回创建好的DataStore实例
     *
     * 3. 生命周期管理：
     *    - @Singleton 确保整个应用只有一个DataStore实例
     *    - 避免重复创建，提高性能并确保数据一致性
     */
}