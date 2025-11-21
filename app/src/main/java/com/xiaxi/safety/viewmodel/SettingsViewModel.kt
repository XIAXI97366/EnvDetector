package com.xiaxi.safety.viewmodel

import android.content.Context
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import dagger.hilt.android.lifecycle.HiltViewModel
import com.xiaxi.safety.datastore.model.Language
import com.xiaxi.safety.repository.AppSettingsRepository
import dagger.hilt.android.qualifiers.ApplicationContext
import kotlinx.coroutines.launch
import javax.inject.Inject

/**
 * 设置界面的 ViewModel，负责管理设置界面的所有业务逻辑和状态
 *
 * 与其他组件的关系：
 * ```
 * UI Layer (Settings Compose)
 *        ↓ 用户交互
 * SettingsViewModel  ← 当前类
 *        ↓ 委托操作
 * AppSettingsRepository
 *        ↓ 数据操作
 * AppSettingsDataStore
 *        ↓ 持久化
 * DataStore + Protocol Buffers
 * ```
 *
 * Hilt依赖注入详解：
 * - @HiltViewModel: 标记此类为 Hilt 管理的 ViewModel
 * - @Inject constructor: 启用构造函数依赖注入
 * - Repository 注入: 自动获取Repository实例
 * - Context 注入: 注入应用级别的Context
 *
 * @param appSettingsRepository 应用设置仓储，处理具体的数据操作
 * @param context 应用上下文，用于访问系统服务和资源
 * @see AppSettingsRepository 数据仓储层
 * @see HiltViewModel Hilt ViewModel注解
 */
@HiltViewModel  // 标记此 ViewModel 支持 Hilt 依赖框架注入，由 Hilt 负责创建和注入它的构造参数
class SettingsViewModel @Inject constructor(  // 构造函数注入
    private val appSettingsRepository: AppSettingsRepository,  // 注入 Repository 依赖
    @ApplicationContext private val context: Context  // 注入应用级Context
) : ViewModel() {

    /**
     * Hilt 在 ViewModel 中的工作原理详解：
     *
     * 1. @HiltViewModel 注解的作用：
     *    - 告诉 Hilt 这是一个需要依赖注入的 ViewModel
     *    - Hilt 会在编译时生成相应的 ViewModelFactory
     *    - 支持通过 hiltViewModel() 或 viewModels() 获取实例
     *    - 自动处理 ViewModel 的创建和依赖注入
     *
     * 2. 构造函数注入机制：
     *    - @Inject constructor 标记构造函数支持依赖注入
     *    - Hilt 会自动提供构造函数参数所需的依赖
     *    - appSettingsRepository 来自 DataStoreModule 提供的依赖
     *    - @ApplicationContext 确保注入的是应用级别的Context
     *    - 所有依赖都在ViewModel创建时自动注入
     *
     * 3. 生命周期管理：
     *    - ViewModel 的生命周期由 ViewModelComponent 管理
     *    - 在 Activity/Fragment 销毁时，ViewModel 会被正确清理
     *    - 依赖的 Repository 等会根据其作用域进行管理
     *    - viewModelScope 会在 ViewModel 清理时自动取消所有协程
     *
     * 4. 作用域层次：
     *    - ApplicationComponent: 应用级别，提供Repository
     *    - ViewModelComponent: ViewModel级别，管理ViewModel实例
     *    - 依赖关系自动解析和注入
     *
     * 5. 使用方式：
     *    ```kotlin
     *    // 在Compose中使用
     *    @Composable
     *    fun SettingsScreen() {
     *        val viewModel: SettingsViewModel = hiltViewModel()
     *        // 使用viewModel...
     *    }
     *
     *    // 在Fragment中使用
     *    class SettingsFragment : Fragment() {
     *        private val viewModel: SettingsViewModel by viewModels()
     *        // 使用viewModel...
     *    }
     *    ```
     */
    // ==================== 设置操作方法 ====================

    /**
     * 设置应用界面语言
     * 执行流程：
     * 1. 在 viewModelScope 中启动协程
     * 2. 委托 Repository 执行实际的语言设置操作
     * 3. Repository 会更新 DataStore 中的语言配置
     * 4. 配置变更会通过响应式数据流通知所有观察者
     */
    fun setLanguage(language: Language) {
        viewModelScope.launch {
            appSettingsRepository.setLanguage(language)
        }
    }
}