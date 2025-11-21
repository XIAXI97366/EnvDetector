package com.xiaxi.safety.ui.navigation

import androidx.annotation.StringRes
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Home
import androidx.compose.material.icons.filled.Settings
import androidx.compose.material.icons.outlined.Home
import androidx.compose.material.icons.outlined.Settings
import androidx.compose.ui.graphics.vector.ImageVector
import com.xiaxi.safety.R

/**
 * 主界面导航枚举
 * 这个枚举定义了应用的主要导航界面，为底部导航栏提供统一的界面配置
 * 每个导航项包含路由信息、显示标签、图标资源等完整的导航配置
 *
 * 导航配置：
 * - route: 导航路由字符串，用于 Navigation Compose
 * - label: 国际化字符串资源ID，显示在底部导航栏
 * - icon: 未选中状态的轮廓图标
 * - iconFilled: 选中状态的填充图标
 *
 * 使用场景：
 * - 底部导航栏的项目配置
 * - Navigation Compose 的路由定义
 * - 导航状态的类型安全检查
 * - 界面标签的国际化显示
 *
 * 扩展方式：
 * 添加新的导航界面只需要在枚举中增加新项：
 * ```kotlin
 * NewScreen(
 *     route = "NewScreen",
 *     label = R.string.new_screen_label,
 *     icon = Icons.Outlined.NewIcon,
 *     iconFilled = Icons.Filled.NewIcon
 * )
 * ```
 *
 * @param route 导航路由字符串，必须唯一
 * @param label 界面标签的字符串资源ID
 * @param icon 未选中状态的图标（轮廓风格）
 * @param iconFilled 选中状态的图标（填充风格）
 *
 * @see androidx.navigation.compose.NavHost 导航主机
 * @see com.reveny.nativecheck.ui.activity.BottomNav 底部导航栏实现
 */
enum class MainScreen(
    val route: String,
    @StringRes val label: Int,
    val icon: ImageVector,
    val iconFilled: ImageVector
) {
    /**
     * 主页界面内容：
     * - 检测结果列表
     * - 设备基本信息
     * - 系统信息展示
     * - 检测状态指示
     */
    Home(
        route = "HomeScreen",
        label = R.string.home_label,
        icon = Icons.Outlined.Home,
        iconFilled = Icons.Filled.Home
    ),

    /**
     * 设置界面提供如下设置分类：
     * - 语言设置：界面语言选择
     */
    Settings(
        route = "SettingsScreen",
        label = R.string.settings_label,
        icon = Icons.Outlined.Settings,
        iconFilled = Icons.Filled.Settings
    )
}