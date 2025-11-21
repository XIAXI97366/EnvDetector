package com.xiaxi.safety.ui.component

import androidx.compose.foundation.ExperimentalFoundationApi
import androidx.compose.foundation.LocalIndication
import androidx.compose.foundation.clickable
import androidx.compose.foundation.combinedClickable
import androidx.compose.foundation.interaction.MutableInteractionSource
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.selection.toggleable
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.MaterialTheme.colorScheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.material3.VerticalDivider
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.painter.Painter
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

/**
 * 基础设置项组件：用于显示带有图标、标题和描述的可点击设置选项
 *
 * @param title 设置项标题
 * @param description 设置项描述文本
 * @param icon 可选的设置项图标
 * @param onClick 点击回调函数
 */
@Composable
fun SettingItem(title: String, description: String, icon: ImageVector?, onClick: () -> Unit) {
    Surface(modifier = Modifier.clickable { onClick() }) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 16.dp, vertical = 20.dp),
            verticalAlignment = Alignment.CenterVertically,
        ) {
            // 可选图标
            icon?.let {
                Icon(
                    imageVector = icon,
                    contentDescription = null,
                    modifier = Modifier
                        .padding(end = 16.dp)
                        .size(24.dp),
                    tint = colorScheme.primary,
                )
            }

            // 文本内容
            Column(
                modifier = Modifier
                    .weight(1f)
                    .padding(start = if (icon == null) 12.dp else 0.dp)
            ) {
                // 标题文本
                Text(
                    text = title,
                    maxLines = 1,
                    style = MaterialTheme.typography.titleLarge,
                    color = colorScheme.onSurface,
                    overflow = TextOverflow.Ellipsis,
                )
                Spacer(modifier = Modifier.height(2.dp))

                // 描述文本
                Text(
                    text = description,
                    color = colorScheme.onSurfaceVariant,
                    maxLines = 2,
                    style = MaterialTheme.typography.bodyMedium,
                    overflow = TextOverflow.Ellipsis,
                )
            }
        }
    }
}

/**
 * 通用设置项组件
 *
 * @param title 设置项标题，必需参数
 * @param description 设置项描述，可选参数
 * @param icon 设置项图标，支持ImageVector或Painter
 * @param enabled 是否启用，控制交互和视觉状态
 * @param onLongClickLabel 长按操作的可访问性标签
 * @param onLongClick 长按回调函数
 * @param onClickLabel 点击操作的可访问性标签
 * @param leadingIcon 前置自定义内容组合函数
 * @param trailingIcon 后置自定义内容组合函数
 * @param onClick 点击回调函数
 */
@OptIn(ExperimentalFoundationApi::class)
@Composable
fun SettingNormalItem(
    title: String,
    description: String? = null,
    icon: Any? = null,
    enabled: Boolean = true,
    onLongClickLabel: String? = null,
    onLongClick: (() -> Unit)? = null,
    onClickLabel: String? = null,
    leadingIcon: (@Composable () -> Unit)? = null,
    trailingIcon: (@Composable () -> Unit)? = null,
    onClick: () -> Unit = {},
) {
    Surface(
        modifier = Modifier.combinedClickable(
            onClick = onClick,
            onClickLabel = onClickLabel,
            enabled = enabled,
            onLongClickLabel = onLongClickLabel,
            onLongClick = onLongClick
        )
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(8.dp, 16.dp),
            verticalAlignment = Alignment.CenterVertically,
        ) {
            // 前置自定义内容
            leadingIcon?.invoke()

            // 图标处理：支持ImageVector和Painter两种类型
            when (icon) {
                is ImageVector -> {
                    Icon(
                        imageVector = icon,
                        contentDescription = null,
                        modifier = Modifier
                            .padding(start = 8.dp, end = 16.dp)
                            .size(24.dp),
                        tint = colorScheme.onSurfaceVariant
                    )
                }

                is Painter -> {
                    Icon(
                        painter = icon,
                        contentDescription = null,
                        modifier = Modifier
                            .padding(start = 8.dp, end = 16.dp)
                            .size(24.dp),
                        tint = colorScheme.onSurfaceVariant
                    )
                }
            }

            // 主要文本内容区域
            Column(
                modifier = Modifier
                    .weight(1f)
                    .padding(horizontal = if (icon == null && leadingIcon == null) 12.dp else 0.dp)
                    .padding(end = 8.dp)
            ) {
                // 标题文本
                Text(
                    modifier = Modifier,
                    text = title,
                    maxLines = 2,
                    style = MaterialTheme.typography.titleLarge.copy(fontSize = 18.sp),
                    overflow = TextOverflow.Ellipsis
                )

                // 可选描述文本
                if (!description.isNullOrEmpty())
                    Text(
                        modifier = Modifier.padding(top = 2.dp),
                        text = description,
                        maxLines = Int.MAX_VALUE,  // 允许多行描述
                        style = MaterialTheme.typography.bodyMedium,
                        color = colorScheme.onSurfaceVariant,
                        overflow = TextOverflow.Ellipsis
                    )
            }

            // 后置内容和分隔线
            trailingIcon?.let {
                // 垂直分隔线
                VerticalDivider(
                    modifier = Modifier
                        .height(32.dp)
                        .padding(horizontal = 8.dp)
                        .align(Alignment.CenterVertically),
                    color = colorScheme.onSurface.copy(alpha = 0.3f),
                    thickness = 1.dp
                )
                // 后置自定义内容
                trailingIcon.invoke()
            }
        }
    }
}

/**
 * 开关设置项组件，提供直观的开关交互和清晰的状态指示
 *
 * @param title 设置项标题
 * @param description 设置项描述，可选
 * @param icon 设置项图标，可选
 * @param enabled 是否启用交互
 * @param isChecked 开关是否选中
 * @param onClick 开关切换回调
 */
@Composable
fun SettingSwitchItem(
    title: String,
    description: String? = null,
    icon: ImageVector? = null,
    enabled: Boolean = true,
    isChecked: Boolean = true,
    onClick: (() -> Unit) = {},
) {
    // 交互源管理，用于统一的交互反馈
    val interactionSource = remember { MutableInteractionSource() }

    Surface(
        modifier = Modifier.toggleable(
            value = isChecked,
            enabled = enabled,
            onValueChange = { onClick() },  // 整行可点击切换
            indication = LocalIndication.current,
            interactionSource = interactionSource
        )
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(8.dp, 16.dp)
                .padding(start = 0.dp),
            verticalAlignment = Alignment.CenterVertically,
        ) {
            // 可选图标
            if (icon != null) {
                Icon(
                    imageVector = icon,
                    contentDescription = null,
                    modifier = Modifier
                        .padding(start = 8.dp, end = 16.dp)
                        .size(24.dp),
                    tint = colorScheme.onSurfaceVariant
                )
            }

            // 文本内容区域
            Column(
                modifier = Modifier.weight(1f)
            ) {
                // 标题文本
                Text(
                    modifier = Modifier,
                    text = title,
                    maxLines = 2,
                    style = MaterialTheme.typography.titleLarge.copy(fontSize = 18.sp),
                    overflow = TextOverflow.Ellipsis
                )

                // 可选描述文本
                if (!description.isNullOrEmpty())
                    Text(
                        modifier = Modifier.padding(top = 2.dp),
                        text = description,
                        maxLines = Int.MAX_VALUE,
                        style = MaterialTheme.typography.bodyMedium,
                        color = colorScheme.onSurfaceVariant,
                        overflow = TextOverflow.Ellipsis
                    )
            }

            // 开关控件
            Switch(
                checked = isChecked,
                onCheckedChange = null,  // 由外层toggleable处理
                interactionSource = interactionSource,  // 共享交互源
                modifier = Modifier.padding(start = 20.dp, end = 6.dp),
                enabled = enabled,
            )
        }
    }
}
