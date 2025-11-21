package com.xiaxi.safety.ui.navigation.graphs

import androidx.compose.animation.fadeIn
import androidx.compose.animation.fadeOut
import androidx.navigation.NavController
import androidx.navigation.NavGraphBuilder
import androidx.navigation.compose.composable
import androidx.navigation.navigation
import com.xiaxi.safety.ui.screens.settings.Settings
import com.xiaxi.safety.ui.navigation.MainScreen
import com.xiaxi.safety.ui.screens.settings.category.App

/**
 * 定义设置模块内的所有导航路由，包括主设置界面和各个设置分类界面
 *
 * 导航层次：
 * ```
 * Settings (主设置界面)
 * ├── App (应用设置)
 * └── [未来可扩展的其他分类]
 * ```
 *
 * 设置分类说明：
 * - Home: 设置主界面，显示所有设置分类的入口
 * - App: 语言、应用相关设置，如遥测、实验功能等
 *
 * 后续扩展示例：
 * ```kotlin
 * enum class SettingsScreen(val route: String) {
 *     Home("Settings"),
 *     App("App"),
 *     Appearance("Appearance"),    // 自定义外观
 *     Language("Language"),        // 语言设置
 *     Privacy("Privacy"),          // 隐私设置
 *     About("About"),              // 关于界面
 * }
 * ```
 *
 * @param route 导航路由字符串，用于 Navigation Compose
 * @see settingsScreen 设置导航图构建函数
 */
enum class SettingsScreen(
    val route: String
) {
    /**
     * 设置主界面:
     * 设置模块的入口界面，显示所有设置分类的列表
     * 用户可以从这里导航到具体的设置分类界面
     */
    Home("Settings"),

    /**
     * 应用设置界面，包含应用相关的设置选项，如：
     * - 语言
     * - 遥测数据收集开关
     * - 实验性功能开关
     * - 应用行为配置
     */
    App("App"),
}

/**
 * 设置导航图构建扩展函数
 * fun NavGraphBuilder.settingsScreen 为 NavGraphBuilder 的扩展函数 ==> 设置导航图构建扩展函数，主要用于构建设置模块的完整导航图
 * 同时负责定义设置模块内所有界面的导航配置，支持多层级的设置界面导航
 *
 * 导航结构：
 * ```
 * MainScreen.Settings (主导航)
 * └── SettingsScreen.Home (设置主界面)
 *     ├── SettingsScreen.App (应用设置)
 *     └── 后续待定
 * ```
 *
 * 界面配置：
 * 1. Settings主界面：
 *    - 显示所有设置分类的入口
 *    - 提供导航到具体设置分类的功能
 *
 * 2. App设置界面：
 *    - 应用行为相关的设置选项
 *    - 实验性功能的开关控制
 *
 * 后续扩展新的设置分类界面步骤：
 * 1. 在 SettingsScreen 枚举中添加新路由
 * 2. 在此函数中添加对应的 composable 配置
 * 3. 创建对应的设置界面 Composable
 * 4. 在 Settings 主界面中添加导航入口
 *
 * @param navController 导航控制器，用于执行导航操作
 * @return Unit 扩展函数无返回值，直接修改 NavGraphBuilder
 *
 * 使用示例：
 * ```kotlin
 * NavHost(navController = navController, startDestination = MainScreen.Home.route) {
 *     homeScreen(navController, viewModel, detections)
 *     settingsScreen(navController)  // 添加设置导航图
 * }
 * ```
 *
 * @see NavGraphBuilder Navigation Compose 图构建器
 * @see Settings 设置主界面 Composable
 * @see App 应用设置界面 Composable
 */
fun NavGraphBuilder.settingsScreen(
    navController: NavController
) = navigation(
    startDestination = SettingsScreen.Home.route,  // 设置模块的默认入口
    route = MainScreen.Settings.route              // 设置模块的根路由
) {
    // 设置主界面配置
    composable(
        route = SettingsScreen.Home.route,
        enterTransition = { fadeIn() },            // 进入时淡入动画
        exitTransition = { fadeOut() }             // 退出时淡出动画
    ) {
        Settings(
            navController = navController           // 传递导航控制器，用于导航到子设置界面
        )
    }

    // 应用设置界面配置
    composable(
        route = SettingsScreen.App.route,
        enterTransition = { fadeIn() },            // 进入时淡入动画
        exitTransition = { fadeOut() }             // 退出时淡出动画
    ) {
        App(navController = navController)         // 应用设置界面
    }
}