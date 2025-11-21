package com.xiaxi.safety.ui.theme

import androidx.compose.material3.Typography
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.style.LineBreak
import androidx.compose.ui.text.style.TextDirection

/**
 * 应用字体排版系统（基于 Material Design 3 规范的完整字体排版系统，为整个应用提供一致的文本样式）
 *
 * Typography层次结构：
 * ```
 * Display (展示文本)
 * ├── displayLarge   - 57sp, 最大的展示文本，用于品牌标题
 * ├── displayMedium  - 45sp, 中等展示文本，用于重要标题
 * └── displaySmall   - 36sp, 小型展示文本，用于次要标题
 *
 * Headline (标题文本)
 * ├── headlineLarge  - 32sp, 大型标题，用于页面主标题
 * ├── headlineMedium - 28sp, 中型标题，用于章节标题
 * └── headlineSmall  - 24sp, 小型标题，用于子章节标题
 *
 * Title (标题)
 * ├── titleLarge     - 22sp, 大型标题，用于卡片标题
 * ├── titleMedium    - 16sp, 中型标题，用于列表项标题
 * └── titleSmall     - 14sp, 小型标题，用于辅助标题
 *
 * Body (正文)
 * ├── bodyLarge      - 16sp, 大型正文，用于重要内容
 * ├── bodyMedium     - 14sp, 中型正文，用于一般内容
 * └── bodySmall      - 12sp, 小型正文，用于辅助说明
 *
 * Label (标签)
 * ├── labelLarge     - 14sp, 大型标签，用于按钮文本
 * ├── labelMedium    - 12sp, 中型标签，用于选项卡
 * └── labelSmall     - 11sp, 小型标签，用于辅助标签
 * ```
 *
 * @see Typography Material Design 3字体排版系统
 * @see TextStyle Compose文本样式
 * @see LineBreak 文本换行策略
 * @see TextDirection 文本方向控制
 */
val Typography =
    Typography().run {
        copy(
            // 正文文本样式 - 应用段落换行和文本方向优化
            bodyLarge = bodyLarge.applyLinebreak().applyTextDirection(),    // 16sp, 重要正文内容
            bodyMedium = bodyMedium.applyLinebreak().applyTextDirection(),  // 14sp, 一般正文内容
            bodySmall = bodySmall.applyLinebreak().applyTextDirection(),    // 12sp, 辅助说明文本

            // 标题文本样式 - 仅应用文本方向优化（标题通常不需要段落换行）
            titleLarge = titleLarge.applyTextDirection(),    // 22sp, 卡片和组件标题
            titleMedium = titleMedium.applyTextDirection(),  // 16sp, 列表项和设置项标题
            titleSmall = titleSmall.applyTextDirection(),    // 14sp, 小型组件标题

            // 页面标题样式 - 用于页面和章节的主要标题
            headlineSmall = headlineSmall.applyTextDirection(),   // 24sp, 页面子标题
            headlineMedium = headlineMedium.applyTextDirection(), // 28sp, 页面主标题
            headlineLarge = headlineLarge.applyTextDirection(),   // 32sp, 重要页面标题

            // 展示文本样式 - 用于品牌和重要信息展示
            displaySmall = displaySmall.applyTextDirection(),   // 36sp, 小型展示文本
            displayMedium = displayMedium.applyTextDirection(), // 45sp, 中型展示文本
            displayLarge = displayLarge.applyTextDirection(),   // 57sp, 大型展示文本

            // 标签文本样式 - 用于按钮、选项卡和标签
            labelLarge = labelLarge.applyTextDirection(),   // 14sp, 按钮文本
            labelMedium = labelMedium.applyTextDirection(), // 12sp, 选项卡文本
            labelSmall = labelSmall.applyTextDirection(),   // 11sp, 小型标签文本
        )
    }

/**
 * 应用段落换行优化的文本样式扩展函数
 *
 * @return 应用了段落换行策略的新TextStyle实例
 */
private fun TextStyle.applyLinebreak(): TextStyle = this.copy(lineBreak = LineBreak.Paragraph)

/**
 * 应用文本方向优化的文本样式扩展函数
 *
 * @return 应用了内容感知文本方向的新 TextStyle 实例
 */
fun TextStyle.applyTextDirection(): TextStyle =
    this.copy(textDirection = TextDirection.Content)