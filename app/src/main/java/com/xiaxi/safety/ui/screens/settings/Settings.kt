package com.xiaxi.safety.ui.screens.settings

import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.material3.ExperimentalMaterial3Api
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
import com.xiaxi.safety.R
import com.xiaxi.safety.ui.component.SettingItem
import com.xiaxi.safety.viewmodel.SettingsViewModel
import compose.icons.TablerIcons
import compose.icons.tablericons.Filled
import compose.icons.tablericons.filled.Settings

/**
 * 设置主界面 Compose 组件，设置模块的入口界面，提供了应用所有设置分类的访问入口
 *
 * 界面结构：
 * ```
 * LargeTopAppBar (设置标题栏)
 * └── LazyColumn (设置项目列表)
 *     ├── SettingItem (应用设置)
 *     ├── SettingItem (外观设置)
 *     └── [未来可扩展的其他设置项]
 * ```
 *
 * ViewModel 集成：
 * - Hilt注入：使用 hiltViewModel() 获取 ViewModel 实例
 * - 状态管理：虽然主界面不直接操作设置，但为扩展预留
 * - 数据传递：为子界面提供统一的数据管理入口
 *
 * 后续添加新的设置分类只需要：
 * 1. 在 LazyColumn 中添加新的 SettingItem
 * 2. 配置对应的导航路由
 * 3. 创建对应的设置子界面
 * 4. 在导航图中注册新路由
 *
 * @param navController 导航控制器，用于导航到各个设置子界面
 * @param viewModel 设置 ViewModel，提供设置相关的数据和操作
 *
 * 导航路由：
 * - "App" → 应用设置界面
 *
 * 使用示例：
 * ```kotlin
 * // 在导航图中使用
 * composable(route = SettingsScreen.Home.route) {
 *     Settings(navController = navController)
 * }
 * ```
 * @see SettingsViewModel 设置界面数据和状态管理
 * @see SettingItem 设置项组件
 * @see App 应用设置界面
 */
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun Settings(
    navController: NavController,
    viewModel: SettingsViewModel = hiltViewModel()  // 使用 Hilt 注入 ViewModel，其类型是 SettingsViewModel
) {
    // 配置顶部栏的滚动行为
    // exitUntilCollapsedScrollBehavior: 滚动时顶部栏会折叠到最小高度
    val scrollBehavior =
        TopAppBarDefaults.exitUntilCollapsedScrollBehavior(
            rememberTopAppBarState(),  // 记住顶部栏状态
            canScroll = { true }       // 允许滚动
        )

    Scaffold(
        modifier = Modifier
            .fillMaxSize()
            // 连接嵌套滚动，使顶部栏能响应内容滚动
            .nestedScroll(scrollBehavior.nestedScrollConnection),
        topBar = {
            // 大型顶部应用栏，显示设置标题
            LargeTopAppBar(
                title = { Text(stringResource(R.string.settings)) },  // 应用 "设置" 标题
                scrollBehavior = scrollBehavior,  // 应用滚动行为
            )
        }
    ) { paddingValues ->
        // 设置项目列表
        LazyColumn(
            modifier = Modifier
                .fillMaxSize()
                .padding(paddingValues),  // 应用 Scaffold 提供的内边距
        ) {
            // App设置->通用
            item {
                SettingItem(
                    title = stringResource(R.string.settings_app),           // 应用通用标题
                    description = stringResource(R.string.settings_app_desc), // 应用设置描述
                    icon = TablerIcons.Filled.Settings,                      // 设置图标
                    onClick = {
                        // 导航到应用设置界面
                        navController.navigate("App")
                    }
                )
            }
            // 后续可以在这里添加更多设置分类
            // 例如：
            // - 语言设置
            // - 隐私设置
            // - 关于界面
            // - 备份恢复
            // - 高级设置
        }
    }
}