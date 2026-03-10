package com.xiaxi.safety.ui.theme

import android.os.Build
import androidx.annotation.RequiresApi
import androidx.compose.material3.LocalTextStyle
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ProvideTextStyle
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.style.LineBreak
import androidx.compose.ui.text.style.TextDirection

/**
 * 应用主题配置组件，负责配置整个应用的视觉风格和颜色方案
 * 兼容性：
 * - Android 12+：完整的Material You支持
 * - 较低版本：需降级到自定义颜色
 *
 * @param themeColor 自定义主题颜色
 */

/** @Composable fun AppTheme(themeColor: Color, content: @Composable () -> Unit)
     这是 Compose 可组合函数，它创建一套主题环境，并在该环境中渲染 content **/

// @RequiresApi(S) 仅在 Android 12+ 支持完整的动态颜色，低版本调用会崩（建议做兼容，见“改进”）
@RequiresApi(Build.VERSION_CODES.S)
@Composable     //  Composable 是一个可组合函数，供 Compose 运行时参与重组
fun AppTheme(
    themeColor: Color,
    content: @Composable () -> Unit
) {
    // 固定浅色配色，仅以 themeColor 为主色
    val colorScheme = lightColorScheme(
        primary = themeColor,
        onPrimary = Color.White,
        primaryContainer = themeColor.copy(alpha = 0.92f),
        secondary = themeColor,
        tertiary = themeColor,
        surfaceTint = themeColor
    )

    // 配置文本样式：提供优化的文本排版和阅读体验
    ProvideTextStyle(
        value = LocalTextStyle.current.copy(
            // 段落级别的换行优化，提供更好的阅读体验
            lineBreak = LineBreak.Paragraph,
            // 内容感知的文本方向，支持RTL语言
            textDirection = TextDirection.Content
        )
    ) {
        // 使用 Material Design 3 主题
        MaterialTheme(
            colorScheme = colorScheme,  // 动态生成的颜色方案
            typography = Typography,    // 自定义字体排版
            content = content          // 子组件内容
        )
    }
}