# MainActivity 解析

```kotlin
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
     * - 提供更现代的视觉体验
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
     * @param savedInstanceState 保存的实例状态，用于恢复Activity状态
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

        // 异步获取用户设置并启动检测任务，使用协程确保不阻塞主线程
        lifecycleScope.launch {
            // 启动Root检测任务，传入必要的参数
            mainViewModel.performTask(this@MainActivity, packageManager)
        }

        /** 创建应用更新检查器，用于检查和下载应用更新，暂未进行支持，先进行注释 **/
        // val updateChecker = UpdateChecker(this)

        
    }
}
```

## appSettingsRepository 字段依赖注入流程

```kotlin
/**
 * App 设置序列化器
 * 这是 DataStore 与 AppSettings 数据模型之间的桥梁，负责处理应用设置的序列化和反序列化操作
 * 将 AppSettings 对象序列化为二进制数据并保存到磁盘
 * 从磁盘读取二进制数据并反序列化为 AppSettings 对象
 */
 /**
 * 该 AppSettingsSerializer 类构造添加的 @Inject 注解，在需要创建 AppSettingsSerializer 对象时
 * 会由 Dagger 编译器生成的 AppSettingsSerializer_Factory 工厂类，Hilt 在运行时用它创建实例
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

/**
 * Hilt 依赖框架注入模块 -> DataStore 配置
 * 该模块负责提供 DataStore 相关的依赖，用于 App 设置的持久化存储
 * 应用全局单例
 */
@Module  // 标记这是一个 Hilt 模块，包含依赖提供方法
@InstallIn(SingletonComponent::class)  // 将此模块"安装到应用级组件"，让该模块里提供的依赖在应用全局范围可被注入
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
        // 创建名为 "app_settings.pb" 数据存储文件，位于 /data/data/com.xiaxi.safety/files/datastore/
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
     *    - 返回创建好的 DataStore 实例
     *
     * 3. 生命周期管理：
     *    - @Singleton 确保整个应用只有一个DataStore实例
     *    - 避免重复创建，提高性能并确保数据一致性
     */
}
																		/\
																		||
																		||
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

    /**
     * 设置 App 主题颜色
     */
    suspend fun setThemeColor(value: ThemeColor) = update { copy(themeColor = value) }
}
																		/\
																		||
																		||
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

    /**
     * 设置 App 的主题颜色
     */
    suspend fun setThemeColor(value: ThemeColor) = userPreferencesDataSource.setThemeColor(value)
}
																		/\
																		||
																		||
@AndroidEntryPoint  // 标记该 MainActivity 类支持 Hilt 依赖框架注入
class MainActivity : AppCompatActivity() {

    /**
     * 通过字段注入获取 AppSettingsRepository
     * @Inject注解 告诉 Hilt 在 Activity 创建时自动注入这个依赖
     * lateinit var 确保在 onCreate 之内完成注入
     */
    @Inject
    lateinit var appSettingsRepository: AppSettingsRepository
}
```
