package com.xiaxi.safety.ui.screens.settings.category

import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.LargeTopAppBar
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.material3.rememberTopAppBarState
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.input.nestedscroll.nestedScroll
import androidx.compose.ui.res.stringResource
import androidx.hilt.navigation.compose.hiltViewModel
import androidx.navigation.NavController
import com.xiaxi.safety.viewmodel.SettingsViewModel
import compose.icons.TablerIcons
import compose.icons.tablericons.Outline
import compose.icons.tablericons.outline.ChevronLeft
import com.xiaxi.safety.R
import com.xiaxi.safety.ui.component.SettingSwitchItem
import com.xiaxi.safety.ui.providable.LocalAppSettings

/**
 * 应用设置界面 Compose 组件
 * 这是应用行为相关设置的专门界面，包含语言（暂未支持：遥测、实验功能）等应用级别的配置选项
 *
 * 核心功能：
 * - 语言设置：应用界面语言的选择和切换
 * - 遥测控制：用户数据收集的开关控制   -> 暂不支持
 * - 实验功能：实验性检测功能的启用控制  -> 暂不支持
 * - 设置同步：实时同步设置变更到数据存储
 * - 返回导航：提供返回到设置主界面的导航
 *
 * 设置项目：
 * - Language：界面语言选择（下拉菜单形式）
 * - 禁用遥测：控制是否收集使用数据和分析信息
 * - 实验功能：启用实验性的检测方法
 *
 * 界面结构：
 * ```
 * LargeTopAppBar (带返回按钮的标题栏)
 * └── LazyColumn (设置项目列表)
 *     ├── Language (语言选择组件)
 *     ├── SettingSwitchItem (遥测开关)     -> 暂不支持
 *     └── SettingSwitchItem (实验功能开关)   -> 暂不支持
 * ```
 *
 * 数据管理：
 * - LocalAppSettings：获取当前应用设置状态
 * - SettingsViewModel：处理设置变更操作
 * - 响应式更新：设置变更自动更新 UI 状态
 * - 持久化存储：通过 DataStore 自动保存设置
 *
 * @param navController 导航控制器，用于返回到上级界面
 * @param viewModel 设置ViewModel，提供设置操作和状态管理
 *
 * 设置操作：
 * - setLanguage: 设置界面语言
 * - setDisableTelemetry: 设置遥测开关    -> 暂不支持
 * - setEnableExperimentalDetections: 设置实验功能开关  -> 暂不支持
 *
 * 使用示例：
 * ```kotlin
 * // 在导航图中使用
 * composable(route = SettingsScreen.App.route) {
 *     App(navController = navController)
 * }
 * ```
 *
 * @see SettingsViewModel 设置界面数据和状态管理
 * @see LocalAppSettings 应用设置的组合本地提供者
 * @see Language 语言选择组件
 * @see SettingSwitchItem 开关设置项组件
 */
@Composable
@OptIn(ExperimentalMaterial3Api::class)
fun App(
    navController: NavController,
    viewModel: SettingsViewModel = hiltViewModel()  // 使用 Hilt 注入 ViewModel，其类型是 SettingsViewModel
) {
    // 配置顶部栏的滚动行为
    val scrollBehavior = TopAppBarDefaults.exitUntilCollapsedScrollBehavior(
        rememberTopAppBarState(),
        canScroll = { true }
    )

    // 获取当前应用设置状态
    val appSettings = LocalAppSettings.current

    Scaffold(
        modifier = Modifier
            .fillMaxSize()
            .nestedScroll(scrollBehavior.nestedScrollConnection),
        topBar = {
            // 带返回按钮的大型顶部栏
            LargeTopAppBar(
                title = { Text(stringResource(R.string.settings_app)) },  // 应用标题
                scrollBehavior = scrollBehavior,
                navigationIcon = {
                    // 返回按钮
                    IconButton(onClick = { navController.popBackStack() }) {
                        Icon(
                            TablerIcons.Outline.ChevronLeft,
                            contentDescription = null  // 返回按钮的语义由 IconButton 提供
                        )
                    }
                },
            )
        }
    ) { paddingValues ->
        // 应用设置项目列表
        LazyColumn(
            modifier = Modifier
                .fillMaxSize()
                .padding(paddingValues)
        ) {
            // 语言设置项
            item {
                Language(
                    language = appSettings.language,           // 当前语言设置
                    onLanguageChange = viewModel::setLanguage  // 语言变更回调
                )
            }
        }
    }
}
