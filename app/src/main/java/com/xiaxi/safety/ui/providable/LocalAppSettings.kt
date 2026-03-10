package com.xiaxi.safety.ui.providable

import androidx.compose.runtime.compositionLocalOf
import com.xiaxi.safety.datastore.model.AppSettings

/**
 * 应用设置的组合本地提供者：
 * Compose 的 CompositionLocal 可以看成一套依赖注入的机制：
 * 在根节点用 CompositionLocalProvider(LocalAppSettings provides 值) 提供一次之后，
 * 树中任何 Composable 都能通过 LocalAppSettings.current 读取同一个 AppSettings 实例，无需层层传参
 *
 * 生命周期：
 * - 提供阶段：在 MainActivity 中通过 CompositionLocalProvider 提供
 * - 使用阶段：子组件通过 LocalAppSettings.current 访问
 * - 更新阶段：DataStore 数据变化时自动更新所有相关组件
 * - 销毁阶段：随着 Compose UI 树的销毁而清理
 *
 * 使用示例：
 *
 * 1. 提供设置数据（在MainActivity中）：
 * ```kotlin
 * CompositionLocalProvider(LocalAppSettings provides userPreferences) {
 *     AppTheme {
 *         MainScreen(viewModel)
 *     }
 * }
 * ```
 *
 * 2. 访问设置数据（在任何子组件中）：
 * ```kotlin
 * @Composable
 * fun MyComponent() {
 *     val appSettings = LocalAppSettings.current
 *     val isDarkTheme = appSettings.darkTheme
 *
 *     // 根据设置应用不同的UI
 *     if (isDarkTheme) {
 *         DarkThemeContent()
 *     } else {
 *         LightThemeContent()
 *     }
 * }
 * ```
 *
 * 3. 条件性设置提供：
 * ```kotlin
 * @Composable
 * fun SettingsScreen() {
 *     val currentSettings = LocalAppSettings.current
 *     val modifiedSettings = currentSettings.copy(darkTheme = true)
 *
 *     CompositionLocalProvider(LocalAppSettings provides modifiedSettings) {
 *         PreviewContent() // 这里会使用修改后的设置
 *     }
 * }
 * ```
 *
 * @see AppSettings 应用设置数据模型
 * @see CompositionLocalProvider Compose 依赖注入提供者
 * @see compositionLocalOf Compose 本地状态创建函数
 */
val LocalAppSettings = compositionLocalOf { AppSettings() }