package com.xiaxi.safety.ui.activity

import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.imePadding
import androidx.compose.foundation.layout.navigationBars
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.NavigationBar
import androidx.compose.material3.NavigationBarItem
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.navigation.NavController
import androidx.navigation.NavDestination.Companion.hierarchy
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.currentBackStackEntryAsState
import androidx.navigation.compose.rememberNavController
import com.xiaxi.safety.ui.navigation.MainScreen
import com.xiaxi.safety.ui.navigation.graphs.homeScreen
import com.xiaxi.safety.ui.navigation.graphs.settingsScreen
import com.xiaxi.safety.viewmodel.MainViewModel

/**
 * 应用主界面 Compose 组件
 * 这是应用的主要 UI 容器，负责管理整个应用的导航结构、加载状态和内容展示
 * 它实现了现代Android应用的标准 UI 模式，包括底部导航、加载指示器和响应式布局
 *
 * 核心功能：
 * - 管理应用的主要导航结构
 * - 显示加载状态和进度指示器
 * - 协调不同界面之间的切换
 * - 处理底部导航栏的交互
 * - 管理窗口插图和系统UI适配
 *
 * 状态管理：
 * - 监听 ViewModel 的加载状态
 * - 响应式收集检测结果数据
 * - 自动更新UI以反映状态变化
 * - 生命周期安全的状态订阅
 *
 * 导航结构：
 * MainScreen (当前组件)
 * ├── 加载状态 (CircularProgressIndicator)
 * └── 主内容 (Scaffold)
 *     ├── 底部导航栏 (BottomNav)
 *     └── 内容区域 (NavHost)
 *         ├── Home界面 (homeScreen)
 *         └── Settings界面 (settingsScreen)
 *
 */

/**
 * Compose 组件（Compose 组件是指用 @Composable 标注的函数（Composable function））
 * 通常被调用的时机位于 setContent { … } 内生成 UI 树（如 Text/Button 等）==> MainActivity 中 onCreate 中的 setContent{...}
 * 通过参数或 ViewModel 提供状态，状态变化时会自动重组（recomposition），也就是重新执行 @Composable 函数，用新的状态重新拼 UI 树
 * 该组件使用了传入的 MainViewModel 作为状态来源
 */
@Composable
fun MainScreen(viewModel: MainViewModel) {
    // 创建导航控制器，管理界面之间的导航
    val navController = rememberNavController()

    // isLoading的 实时状态来自于 viewModel 对象的 isLoading 所提供
    // 此处的 by 不属于 kotlin 的惰性委托，而是把 Compose 的 State<T> 作为“属性委托”来解包其 .value
    val isLoading by viewModel.isLoading.collectAsState()

    // detections 的实时状态来自于 viewModel 中的 detections 所提供
    // 此处的 by 不属于 kotlin 的惰性委托，而是把 Compose 的 State<T> 作为“属性委托”来解包其 .value
    val detections by viewModel.detections.collectAsState()

    if (isLoading) {
        // 显示加载界面
        // 当检测任务正在执行时，显示居中的圆形进度指示器
        // Box 是 Jetpack Compose 的基础布局容器，功能类似 Android 的 FrameLayout
        Box(
            modifier = Modifier.fillMaxSize(),
            contentAlignment = Alignment.Center
            // 铺满全屏（fillMaxSize），并把子内容居中（contentAlignment.Center）
        ) {
            // 该段代码为 Material3 的圆形加载指示器（转圈圈），用于表示正在加载或进行中
            // 因未使用 progress 没有指定进度，所以会一直旋转动画，直到该 Composable 被移除
            CircularProgressIndicator(
                // 指定用主题的主色绘制，确保与全局主题一致
                color = MaterialTheme.colorScheme.primary
            )
        }
    } else {
        // Scaffold 是 Jetpack Compose Material3 的页面脚手架容器，用来快速搭出常见的页面骨架并统一处理内边距与层级
        // 提供的槽位（slots）：topBar、bottomBar、floatingActionButton、snackbarHost、drawerContent、content
        // 显示主要内容
        // 加载完成后显示带有底部导航的主界面
        Scaffold(
            bottomBar = {
                // 底部导航栏，提供主要界面之间的切换
                BottomNav(navController = navController)
            },
            // 配置窗口插图，确保内容不被系统导航栏遮挡
            // 给内容区域自动加上"等同导航栏"的内边距，避免内容被系统导航栏（常在底部）遮挡
            contentWindowInsets = WindowInsets.navigationBars
        ) {
            // paddingValues：来自你传给 Scaffold 的参数，也就是 bottomBar 和 contentWindowInsets
            // Scaffold 会把系统窗口安全区和自身栏位（如 topBar / bottomBar）转换成一个 PaddingValues 传进来， 用它给内容区留出内边距
            // 可用 Modifier.padding(paddingValues) 或单独取 calculateBottomPadding() 等
            paddingValues ->
            // 导航主机，管理不同界面的显示和切换
            // NavHost：来自 Jetpack Navigation Compose（androidx.navigation.compose.NavHost）提供的导航容器
            // 根据 navController（导航控制器） 的当前目的地显示对应的 Composable 组件
            // 并用如 homeScreen()、settingsScreen()建立路由关系
            NavHost(
                // 给 NavHost 的底部加上内边距，避免内容被系统导航栏和 Scaffold 的 bottomBar 遮挡
                // modifier = Modifier：从一个空的 Modifier 开始
                // .padding(bottom = ...)：只给底部加内边距（不影响上/左/右）
                // paddingValues：Scaffold 传进来的“安全区+栏位”内边距集合，已综合了系统 navigationBars 和 bottomBar 的高度
                // calculateBottomPadding()：从 paddingValues 中取出应该留出的底部 dp 值
                // 效果：当有底部系统栏或 bottomBar 时，NavHost 内容会上移显示；没有时该值通常为 0，不产生额外间距
                modifier = Modifier
                    .padding(bottom = paddingValues.calculateBottomPadding()),
                navController = navController,
                startDestination = MainScreen.Home.route,  // 默认显示 Home 主界面
                // 该 MainScreen 枚举有两个导航界面，Home 界面和 Settings 界面，位于路径 com.xiaxi.safety.ui.navigation.Main.kt
            ) {
                // homeScreen(...):
                // 注册 Home 模块的路由子图，设置该模块的起始页为 HomeScreen.Home.route
                // 并把 navController、MainViewModel 类型的 viewModel 对象、detections 传给实际的 Home(...) 界面，同时配置进入/退出动画等
                // 之所以可以在 NavHost 直接调用 homeScreen 而不是通过 NavGraphBuilder.homeScreen 该方式调用
                // 是因为 NavHost { ... } 的最后一个参数本质是 NavGraphBuilder.() -> Unit
                // 而又因为 Kotlin 支持尾随 Lambda 语法：当最后一个参数是函数类型时，可以把它的实参写在括号外
                // 而 homeScreen(...) 和 settingsScreen(...) 通常都是扩展函数
                // 因此在 NavHost(...){ ... } 的花括号内，可以调用任意多次这些扩展函数，依次向同一个 NavGraphBuilder 添加多个目的地
                homeScreen(
                    navController = navController,
                    viewModel = viewModel,
                    detections = detections  // 传递检测结果数据
                )
                // settingsScreen(...): 注册 Settings 模块的路由子图
                // 包含 Settings 主页及其子页（如 App、Appearance），把 navController 传给这些界面并配置动画
                // Settings 界面路由
                settingsScreen(navController = navController)
            }
        }
    }
}

/**
 * 这是一个 Material Design 3 风格的底部导航栏组件，提供应用主要界面之间的快速切换，支持图标高亮、文本标签和流畅的导航动画
 *
 * 交互逻辑：
 * - 监听当前导航状态
 * - 高亮显示当前选中的界面
 * - 点击时切换到对应界面
 * - 避免重复导航到当前界面
 *
 * 导航项目：
 * - 通过 MainScreen 枚举定义所有导航项
 * - 每个项目包含图标、填充图标、标签和路由
 * - 支持动态添加或移除导航项
 *
 * @param navController 导航控制器，用于执行导航操作
 *
 * 技术实现：
 * - 使用 currentBackStackEntryAsState 监听导航状态
 * - 通过 hierarchy 检查当前选中状态
 * - 使用 NavigationBarItem 提供标准交互
 *
 * BottomNav 在 MainScreen.kt 中是一个顶层私有的 Composable 函数（同时标记了 @Composable 的该函数也为 Compose 组件）
 * 顶层的含义为：函数是直接写在 .kt 文件最外层的，不在任何 class / object / interface
 * private 的顶层函数：作用域是在当前文件内，只有同一个 MainScreen.kt 里的代码能调用它，其他文件看不到
 * 若是 internal/public 顶层函数，可以在同模块或全工程任意地方导入调用
 */
@Composable
private fun BottomNav(
    navController: NavController,
) {
    // 通过 currentBackStackEntryAsState() 把导航控制器当前的 BackStackEntry 转成 State<NavBackStackEntry?>，再用 by 解包成 navBackStackEntry
    // 当导航目的地变化时，这个 State 会更新，从而触发 Composable 重组
    val navBackStackEntry by navController.currentBackStackEntryAsState()
    // navBackStackEntry?.destination 是一个 NavDestination ，表示该回退栈条目对应的“导航图中的节点”（当前所处的目的地/页面或所属的父图）
    // 也就是用它来判断当前在 Home 还是 Settings，以决定底部导航条哪一个 item 处于选中高亮状态
    val currentDestination = navBackStackEntry?.destination

    // 获取所有主界面定义
    // 列出应用底部导航的顶级页面（如 MainScreen 枚举中 Home、Settings 主界面），每项包含 route、label、icon、iconFilled，用于构建底部导航并导航到对应子图
    val mainScreens = MainScreen.entries.toTypedArray()

    // Scaffold 中的 bottomBar 是位置槽位（容器），NavigationBar 是放进去的具体底部导航 UI
    NavigationBar(
        // 添加键盘内边距，确保键盘弹出时导航栏位置正确
        modifier = Modifier.imePadding()
    ) {
        // 遍历所有主界面，创建导航项
        mainScreens.forEach { screen ->
            // 检查当前界面是否被选中
            val selected = currentDestination?.hierarchy?.any { it.route == screen.route } == true
            NavigationBarItem(
                icon = {
                    Icon(
                        // 根据选中状态显示不同图标（填充或轮廓）
                        imageVector = if (selected) screen.iconFilled else screen.icon,
                        contentDescription = null,
                    )
                },
                label = {
                    Text(
                        // 使用国际化字符串资源
                        text = stringResource(id = screen.label),
                        style = MaterialTheme.typography.labelMedium
                    )
                },
                alwaysShowLabel = true,  // 始终显示文本标签
                selected = selected,     // 设置选中状态
                onClick = {
                    // 只有在未选中时才执行导航，避免重复导航
                    if (!selected) navController.navigate(screen.route)
                }
            )
        }
    }
}