package com.xiaxi.safety.ui.activity

import android.os.Build
import android.os.Bundle
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.viewModels
import androidx.appcompat.app.AppCompatActivity
import androidx.compose.runtime.CompositionLocalProvider
import androidx.compose.runtime.getValue
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.lifecycle.lifecycleScope
import dagger.hilt.android.AndroidEntryPoint
import kotlinx.coroutines.launch
import javax.inject.Inject
import com.xiaxi.safety.ui.theme.AppTheme
import com.xiaxi.safety.repository.AppSettingsRepository
import com.xiaxi.safety.ui.providable.LocalAppSettings
import com.xiaxi.safety.viewmodel.MainViewModel

/**
 * 主Activity
 * 生命周期管理：
 * - onCreate: 初始化核心组件和数据
 * - 自动处理配置变更和状态保存
 * - 与ViewModel生命周期协调
 * - 响应系统主题变化
 */
@AndroidEntryPoint  // 标记该 MainActivity 类支持 Hilt 依赖框架注入
class MainActivity : AppCompatActivity() {

    /**
     * 通过字段注入获取 AppSettingsRepository
     * @Inject注解 告诉 Hilt 在 Activity 创建时自动注入这个依赖
     * lateinit var 确保在 onCreate 之内完成注入
     */
    @Inject
    lateinit var appSettingsRepository: AppSettingsRepository

    /**
     * 通过 viewModels() 委托获取 MainViewModel
     * 由于 com.xiaxi.safety.viewmodel.MainViewModel 类没有构造函数参数
     * 会由默认的 ViewModelProvider.Factory，通过无参构造反射创建 MainViewModel
     * viewModels() 是 Android 架构组件提供的委托，确保 ViewModel 的正确生命周期管理
     * 因 by viewModels() 是惰性委托，所以 mainViewModel 对象的构造时机处于第一次用到该对象的时候
     */
    private val mainViewModel: MainViewModel by viewModels()

    /**
     * Activity 生命周期的核心初始化方法
     * 执行流程：
     * 1. 调用父类 onCreate，完成基础初始化
     * 2. 启用边到边显示，提供沉浸式体验
     * 3. 配置导航栏对比度（Android 10+）
     * 4. 初始化 ViewModel 数据
     * 5. 获取用户设置并启动检测任务
     * 6. 创建更新检查器
     * 7. 设置 Compose UI内容
     *
     * 边到边显示配置：
     * - enableEdgeToEdge(): 启用全屏沉浸式体验
     * - isNavigationBarContrastEnforced: 控制导航栏对比度
     *
     * 数据初始化：
     * - 同步初始化设备和应用信息
     * - 异步获取用户设置
     * - 根据设置启动检测任务
     *
     * Compose UI设置：
     * - 响应式数据绑定
     * - 主题配置和个性化
     * - 组合本地提供者设置
     *
     * @param savedInstanceState 保存的实例状态，用于恢复 Activity 状态
     */
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // 启用边到边显示，提供沉浸式用户体验
        enableEdgeToEdge()

        // Android 10+: 禁用导航栏对比度强制，提供更好的视觉效果
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            window.isNavigationBarContrastEnforced = false
        }

        // 初始化 ViewModel 数据：收集设备信息、应用信息、应用签名等
        mainViewModel.initializeData(this)

        // 获取包管理器，用于相关应用检测
        val packageManager = packageManager

        // 异步获取用户设置并启动检测任务（非实时性检测项），使用协程确保不阻塞主线程
        lifecycleScope.launch {
            // 启动检测任务，传入必要的参数
            mainViewModel.performTask(this@MainActivity, packageManager)
        }

        /** 后续添加实时性检测任务 */

        // 设置 Compose UI 内容
        setContent {
            // 收集用户偏好设置的响应式数据流，使用 collectAsStateWithLifecycle 确保生命周期安全
            // appSettingsRepository.data 的类型还是 Flow<AppSettings>
            // collectAsStateWithLifecycle(...) 的返回类型是 State<T>
            // State<T> 定义了只读属性 val value: T，表示当前的最新值，将 Flow<AppSettings> 转换为了 State<AppSettings?>
            // 而 userPreferences 经过 by 把 State 的 value 解包，所以 userPreferences 的类型是 AppSettings?
            val userPreferences by appSettingsRepository.data.collectAsStateWithLifecycle(
                initialValue = null,  // 初始值为 null，等待数据加载，为函数 collectAsStateWithLifecycle(...) 的具名参数
                lifecycle = lifecycle  // 绑定 Activity 生命周期，为函数 collectAsStateWithLifecycle(...) 的具名参数
            )

            // 等待用户偏好数据加载完成
            // 如果左侧不为 null，取左侧，否则执行右侧表达式 ==> userPreferences 为 null，则直接结束 setContent { ... } 这个 lambda 表达式
            userPreferences ?: return@setContent

            // 提供本地组合数据，使子组件可以访问用户设置
            // !! 是 Kotlin 的“非空断言运算符”，把可空值强制当作非空使用；如果实际为 null 则立刻抛出 KotlinNullPointerException（NPE）
            // 等价伪码：x!! -> if (x == null) throw NPE else x
            // LocalAppSettings 的声明定义是 compositionLocalOf { AppSettings() }，同时 里面的 AppSettings() 只是一个默认值占位
            // 当 CompositionLocalProvider 没有提供真实值时，才会用这个默认 AppSettings()，保证程序不至于崩溃
            // 而真正记录和反映用户当前设置的数据，是从 DataStore 读出来的 userPreferences（Flow<AppSettings> → State<AppSettings?> → userPreferences!!）
            // 所以在 LocalAppSettings provides userPreferences!! 中把 LocalAppSettings 的当前值绑定为真正记录和反映用户当前设置的数据的 userPreferences!! 实例
            CompositionLocalProvider(LocalAppSettings provides userPreferences!!) {
                // 应用主题配置，根据用户设置动态调整
                AppTheme(
                    // 根据用户选择设置主题颜色
                    themeColor = userPreferences!!.themeColor.color
                ) {
                    // 主界面内容
                    MainScreen(mainViewModel)
                }
            }
        }
    }
}

