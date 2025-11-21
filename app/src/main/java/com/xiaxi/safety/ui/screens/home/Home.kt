package com.xiaxi.safety.ui.screens.home

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
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
import androidx.compose.ui.unit.dp
import androidx.navigation.NavController
import com.xiaxi.safety.R
import com.xiaxi.safety.app.DetectionResult
import com.xiaxi.safety.ui.component.CheckCard
import com.xiaxi.safety.ui.component.DetectionCard
import com.xiaxi.safety.ui.component.InfoCards
import com.xiaxi.safety.viewmodel.MainViewModel
/**
 * 主页界面 Compose 组件：应用的核心界面，展示检测结果和设备信息
 *
 * 界面结构：
 * ```
 * LargeTopAppBar (可折叠的大型顶部栏)
 * └── LazyColumn (可滚动的内容列表)
 *     ├── CheckCard (检测状态总览)
 *     ├── DetectionCard[] (具体检测项目列表)
 *     └── InfoCards (系统和应用信息)
 * ```
 *
 * 数据展示：
 * - 检测状态：总体检测结果（正常/异常）
 * - 检测详情：每个检测项目的名称和描述
 * - 系统信息：设备型号、Android版本、内核版本
 * - 应用信息：版本、签名、功能状态
 *
 * @param navController 导航控制器，用于界面导航（虽然主页暂时不需要导航）
 * @param viewModel 主界面 ViewModel，提供界面数据和状态管理
 * @param detections 检测结果数据列表，包含所有检测项目的详细信息
 *
 * 数据流：
 * ```
 * MainViewModel → detections (检测结果)
 *              → deviceInfo, androidVersion, kernelVersion (系统信息)
 *              → appVersion, signature, signatureValid (应用信息)
 * ```
 *
 * 使用示例：
 * ```kotlin
 * // 在导航图中使用
 * composable(route = HomeScreen.Home.route) {
 *     Home(
 *         navController = navController,
 *         viewModel = viewModel,
 *         detections = detections
 *     )
 * }
 * ```
 *
 * @see MainViewModel 主界面数据和状态管理
 * @see DetectionResult 检测项的数据
 * @see CheckCard 检测状态卡片组件
 * @see DetectionCard 检测详情卡片组件
 * @see InfoCards 信息卡片容器组件
 */

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun Home(navController: NavController, viewModel: MainViewModel, detections: List<DetectionResult>) {
    // 配置顶部栏的滚动行为
    // exitUntilCollapsedScrollBehavior: 滚动时顶部栏会折叠到最小高度
    val scrollBehavior = TopAppBarDefaults.exitUntilCollapsedScrollBehavior(
        rememberTopAppBarState(),  // 记住顶部栏状态
        canScroll = { true }       // 允许滚动
    )

    Scaffold(
        modifier = Modifier
            .fillMaxSize()
            // 连接嵌套滚动，使顶部栏能响应内容滚动
            .nestedScroll(scrollBehavior.nestedScrollConnection),
        topBar = {
            // 大型顶部应用栏，支持折叠效果
            LargeTopAppBar(
                title = { Text(stringResource(R.string.app_name)) },  // 应用名称作为标题
                scrollBehavior = scrollBehavior  // 应用滚动行为
            )
        },
        // 清除默认的窗口插图，让内容可以延伸到边缘
        contentWindowInsets = WindowInsets(0, 0, 0, 0)
    ) { paddingValues ->
        // LazyColumn 是 Compose 的“懒加载纵向列表”组件，只渲染屏幕可见的项，随滚动按需加载，性能优于一次性渲染所有子项的 Column
        // 用途：实现可滚动长列表（类似 RecyclerView 的纵向列表），无需 Adapter，声明式写法
        // 可滚动的内容列表
        LazyColumn(
            modifier = Modifier
                .fillMaxSize()
                .padding(paddingValues),  // 应用Scaffold提供的内边距
            contentPadding = PaddingValues(16.dp),  // 内容的内边距
            verticalArrangement = Arrangement.spacedBy(16.dp),  // 项目间距
        ) {
            // 检测状态总览卡片
            item {
                CheckCard(
                    // 根据检测结果列表是否为空来判断是否检测到异常
                    isDetected = detections.isNotEmpty(),
                )
            }

            // 具体检测项目列表：使用 items 函数为每个检测项目创建一个 DetectionCard
            items(detections) { detection ->
                DetectionCard(detection = detection)
            }

            // 系统和应用信息卡片
            item {
                InfoCards(viewModel)
            }
        }
    }
}