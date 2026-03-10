package com.xiaxi.safety.datastore.model

import androidx.compose.ui.graphics.Color

enum class ThemeColor(val hex: String) {
    /**
     * 玛莎拉酒红色 - 深沉优雅的红棕色
     */
    Marsala("6D2E46"),

    /**
     * 数字薰衣草色 - 柔和的粉紫色
     */
    DigitalLavender("E4BAD4"),

    /**
     * 宁静蓝色 - 平静的天蓝色
     */
    TranquilBlue("88C1E6"),

    /**
     * 鼠尾草绿色 - 自然的灰绿色
     */
    SageGreen("7A9D7E"),

    /**
     * 网络酸橙色 - 鲜亮的荧光绿
     */
    CyberLime("CFFF04"),

    /**
     * 金麒麟色 - 经典的金黄色
     */
    Goldenrod("DAA520"),

    /**
     * 珊瑚爆裂色 - 活力的橙红色
     */
    CoralBurst("FF6F61"),

    /**
     * 暖灰褐色 - 温暖的中性色（默认主题色）
     */
    WarmTaupe("8B7D6B");

    /**
     * 获取对应的 Compose Color 对象
     * 将十六进制颜色值转换为 Jetpack Compose 可用的 Color 对象
     *
     * @return 对应的 Compose Color 对象
     * @throws IllegalArgumentException 如果颜色格式无效
     */
    val color: Color get() = Color(android.graphics.Color.parseColor("#$hex"))
}

enum class Language {
    /**
     * 跟随系统语言设置
     * 应用会自动检测设备的系统语言设置，
     * 并使用对应的本地化资源。如果系统语言
     * 不在支持列表中，则fallback到英语
     */
    SYSTEM_DEFAULT,

    /**
     * 英语 (English)，对应资源文件：values/strings.xml
     */
    ENGLISH,

    /**
     * 简体中文 (简体中文)，对应资源文件：values-zh-rCN/strings.xml
     */
    CHINESE_SIMPLIFIED,
}
