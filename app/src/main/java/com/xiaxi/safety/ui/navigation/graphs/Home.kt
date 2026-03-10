package com.xiaxi.safety.ui.navigation.graphs

import androidx.compose.animation.fadeIn
import androidx.compose.animation.fadeOut
import androidx.navigation.NavController
import androidx.navigation.NavGraphBuilder
import androidx.navigation.compose.composable
import androidx.navigation.navigation
import com.xiaxi.safety.app.DetectionResult
import com.xiaxi.safety.ui.screens.home.Home
import com.xiaxi.safety.ui.navigation.MainScreen
import com.xiaxi.safety.viewmodel.MainViewModel

/**
 * 主页导航图枚举：用于定义主页模块内的所有导航路由（也就是 Home 模块）
 *
 * 后续扩展示例：
 * ```kotlin
 * enum class HomeScreen(val route: String) {
 *     Home("Home"),
 *     Details("Details"),    // 详情界面
 *     History("History"),    // 历史记录
 * }
 * ```
 *
 * @param route 导航路由字符串，用于 Navigation Compose
 * @see homeScreen 主页导航图构建函数
 */
enum class HomeScreen(
    val route: String
) {
    /**
     * 主页界面
     * 应用的核心功能界面，显示检测结果和设备信息
     * 这是主页模块的默认入口界面
     */
    Home("Home"),
}

/**
 * fun NavGraphBuilder.homeScreen 为 NavGraphBuilder 的扩展函数 ==> 主页导航图构建扩展函数，主要用于构建主页模块的完整导航图
 * 同时负责定义主页模块内所有界面的导航配置，包括路由、动画和数据传递
 *
 * @param navController 导航控制器，用于执行导航操作
 * @param viewModel 主界面 ViewModel，提供数据和状态管理
 * @param detections 检测结果数据列表
 * @return Unit 扩展函数无返回值，直接修改 NavGraphBuilder
 *
 * 使用示例：
 * ```kotlin
 * NavHost(navController = navController, startDestination = MainScreen.Home.route) {
 *     homeScreen(navController, viewModel, detections)
 *     settingsScreen(navController)
 * }
 * ```
 * @see NavGraphBuilder Navigation Compose 图构建器
 * @see MainViewModel 主界面数据和状态管理
 * @see DetectionResult 检测结果
 * @see Home 主页界面 Composable
 */
fun NavGraphBuilder.homeScreen(
    navController: NavController,
    viewModel: MainViewModel,
    detections: List<DetectionResult>
) = navigation(
    startDestination = HomeScreen.Home.route,  // 主页模块的默认入口
    route = MainScreen.Home.route              // 主页模块的根路由
) {
    // 主页界面配置
    composable(
        route = HomeScreen.Home.route,
        enterTransition = { fadeIn() },        // 进入时淡入动画
        exitTransition = { fadeOut() }         // 退出时淡出动画
    ) {
        // 主页界面组合函数，UI函数
        Home(
            navController = navController,      // 传递导航控制器
            viewModel = viewModel,              // 传递 ViewModel
            detections = detections             // 传递检测结果数据
        )
    }
}