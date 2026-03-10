package com.xiaxi.safety.ui.screens.settings.category

import android.annotation.SuppressLint
import android.content.Intent
import androidx.appcompat.app.AppCompatDelegate
import androidx.compose.foundation.layout.Box
import androidx.compose.material.DropdownMenuItem
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.core.os.LocaleListCompat
import com.xiaxi.safety.R
import com.xiaxi.safety.datastore.model.Language
import com.xiaxi.safety.ui.component.SettingNormalItem
import compose.icons.TablerIcons
import compose.icons.tablericons.Outline
import compose.icons.tablericons.outline.Language
import java.util.Locale

/**
 * 支持的语言列表
 *
 * 包含应用支持的所有语言选项，每个语言项包含枚举值、区域设置和显示文本。
 */
private val Languages = LanguageItem.entries.toTypedArray()

/**
 * 下拉菜单形式的语言选择组件，允许用户选择应用的界面语言
 *
 * @param language 当前选择的语言
 * @param onLanguageChange 语言变更回调函数
 */
@Composable
internal fun Language(
    language: Language,
    onLanguageChange: (Language) -> Unit,
) {
    // 下拉菜单展开状态
    var expanded by remember { mutableStateOf(false) }
    val context = LocalContext.current

    Box {
        // 语言设置项，点击时展开下拉菜单
        SettingNormalItem(
            title = stringResource(R.string.pref_language),
            icon = TablerIcons.Outline.Language,
            // 显示当前选择的语言
            description = Languages.find { it.value == language }?.text?.let { stringResource(it) },
            onClick = { expanded = true }
        )

        // 语言选择下拉菜单
        DropdownMenu(expanded = expanded, onDismissRequest = { expanded = false }) {
            Languages.forEach { item ->
                DropdownMenuItem(onClick = {
                    // 保存语言设置
                    onLanguageChange(item.value)

                    // 应用系统语言设置
                    val localeList = LocaleListCompat.create(item.locale)
                    AppCompatDelegate.setApplicationLocales(localeList)
                    expanded = false

                    // 重启应用以使语言变更生效
                    val intent = context.packageManager.getLaunchIntentForPackage(context.packageName)
                    intent?.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP)
                    context.startActivity(intent)
                }) {
                    Text(stringResource(item.text))
                }
            }
        }
    }
}

/**
 * 语言项目枚举
 *
 * 定义应用支持的所有语言选项，每个选项包含：
 * - value: Language枚举值，用于数据存储
 * - locale: Java Locale对象，用于系统语言设置
 * - text: 字符串资源ID，用于界面显示
 *
 * @param value 对应的Language枚举值
 * @param locale 对应的系统Locale对象
 * @param text 显示文本的字符串资源ID
 */
private enum class LanguageItem(
    val value: Language,
    val locale: Locale?,
    val text: Int,
) {
    @SuppressLint("ConstantLocale")
    ENGLISH(Language.ENGLISH, Locale("en"), R.string.en),
    CHINESE_SIMPLIFIED(Language.CHINESE_SIMPLIFIED, Locale("zh", "CN"), R.string.zh_cn),
}
