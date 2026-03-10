package com.xiaxi.safety.ui.component

import android.widget.Toast
import androidx.compose.animation.animateContentSize
import androidx.compose.animation.core.Spring
import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.animation.core.spring
import androidx.compose.foundation.clickable
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowDropDown
import androidx.compose.material.icons.filled.Info
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.getValue
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.rotate
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.platform.LocalClipboardManager
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.AnnotatedString
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.xiaxi.safety.R
import com.xiaxi.safety.app.DetectionResult
import com.xiaxi.safety.datastore.model.ThemeColor
import com.xiaxi.safety.viewmodel.MainViewModel

/**
 * 检测状态卡片组件
 *
 * 状态显示：
 * - 异常状态：显示"环境异常"和相关描述
 * - 正常状态：显示"环境正常"和相关描述
 * - 根据检测结果动态切换文本内容
 *
 * @param modifier 修饰符，用于自定义布局和样式
 * @param isDetected 检测结果，true 表示检测正常，false 表示检测异常
 */
@Composable
fun CheckCard(
    modifier: Modifier = Modifier,
    isDetected: Boolean,
) {
    val backgroundColor =
        if (isDetected) {
            ThemeColor.CoralBurst.color   // 检测异常背景色 → 珊瑚爆裂色
        } else {
            ThemeColor.SageGreen.color    // 检测正常背景色 → 鼠尾草绿色
        }

    Surface(
        modifier = Modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(16.dp))  // 圆角裁剪
            .then(modifier),
        color = backgroundColor,
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 16.dp, vertical = 22.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            // 信息图标
            Icon(
                imageVector = Icons.Filled.Info,
                contentDescription = "Info",
                tint = MaterialTheme.colorScheme.onPrimary,  // 确保在主题色背景上可见
                modifier = Modifier.size(24.dp)
            )
            Spacer(modifier = Modifier.width(24.dp))

            // 文本内容
            Column {
                // 主标题：根据检测结果显示不同文本，检测结果文本为纯白色
                Text(
                    text = if (isDetected)
                        stringResource(R.string.the_environment_is_abnormal)
                    else
                        stringResource(R.string.the_environment_is_normal),
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.onPrimary,
                    fontWeight = FontWeight.Medium
                )

                // 描述文本：提供更详细的说明，详细说明文本为纯白色
                Text(
                    text = if (isDetected)
                        stringResource(R.string.description_abnormal)
                    else
                        stringResource(R.string.description_normal),
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onPrimary
                )
            }
        }
    }
}

/**
 * 检测详情卡片组件（可展开的卡片组件），于显示单个检测项目的详细信息
 *
 * @param detection 检测数据（包含名称和描述信息）
 *
 * @see DetectionResult 检测数据模型
 * @see animateContentSize Compose动画API
 */
@Composable
fun DetectionCard(
    detection: DetectionResult,
) {
    // 展开状态管理
    var expanded by remember { mutableStateOf(false) }
    // 剪贴板管理器，用于复制功能
    val clipboardManager = LocalClipboardManager.current
    // 上下文，用于显示Toast
    val context = LocalContext.current

    Surface(
        modifier = Modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(16.dp))
            .clickable { expanded = !expanded },  // 点击切换展开状态
        color = MaterialTheme.colorScheme.surface,
        tonalElevation = 2.dp  // 轻微阴影效果
    ) {
        Column(
            Modifier
                .padding(all = 16.dp)
                .animateContentSize(  // 内容大小变化动画
                    spring(stiffness = Spring.StiffnessLow)  // 低刚度弹簧动画
                )
        ) {
            // 标题行：包含检测名称和展开指示器
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                // 检测项目名称
                Text(
                    text = detection.name,
                    style = MaterialTheme.typography.titleMedium,
                    modifier = Modifier.weight(1f)  // 占据剩余空间
                )

                // 展开指示器：旋转的下拉箭头
                val rotateAngle by animateFloatAsState(
                    targetValue = if (expanded) 180f else 0f,  // 展开时旋转180度
                    label = "rotateAngle"
                )
                Icon(
                    imageVector = Icons.Filled.ArrowDropDown,
                    contentDescription = null,
                    modifier = Modifier
                        .rotate(rotateAngle)  // 应用旋转动画
                        .alpha(1f)
                        .clickable { expanded = !expanded }  // 图标也可点击
                )
            }

            // 详情内容：只在展开时显示
            if (expanded) {
                Column(
                    Modifier
                        .padding(top = 16.dp)
                        .pointerInput(Unit) {
                            // 检测长按手势
                            detectTapGestures(
                                onLongPress = {
                                    // 复制详情到剪贴板
                                    clipboardManager.setText(AnnotatedString(detection.description))
                                    Toast
                                        .makeText(
                                            context,
                                            "Details copied to clipboard",  // 复制成功提示
                                            Toast.LENGTH_SHORT
                                        )
                                        .show()
                                }
                            )
                        }
                ) {
                    // 详情标题
                    Text(
                        text = stringResource(R.string.detail),
                        style = MaterialTheme.typography.titleSmall
                    )
                    Spacer(modifier = Modifier.height(2.dp))

                    // 详情内容
                    Text(
                        text = detection.description,
                        style = MaterialTheme.typography.bodyMedium
                    )
                }
            }
        }
    }
}

/**
 * 系统信息卡片组件
 *
 * 显示设备的基本系统信息，包括设备型号、Android版本和内核版本。
 * 这些信息对于Root检测的上下文理解很重要。
 *
 * 显示内容：
 * - 设备信息：设备型号和制造商
 * - Android版本：系统版本号
 * - 内核版本：Linux内核版本信息
 *
 * @param deviceInfo 设备信息字符串
 * @param androidVersion Android版本信息
 * @param kernelVersion 内核版本信息
 */
@Composable
fun SystemInfoCard(deviceInfo: String, androidVersion: String, kernelVersion: String) = Surface(
    modifier = Modifier
        .fillMaxWidth()
        .clip(RoundedCornerShape(16.dp)),
    color = MaterialTheme.colorScheme.surface,
    tonalElevation = 1.dp
) {
    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(20.dp),
        verticalArrangement = Arrangement.Center
    ) {
        // 系统信息标题
        Text(
            text = stringResource(R.string.sysinfo),
            style = MaterialTheme.typography.titleSmall,
        )
        Spacer(modifier = Modifier.height(8.dp))

        // 设备信息
        Text(
            text = deviceInfo,
            style = MaterialTheme.typography.bodyMedium
        )
        Spacer(modifier = Modifier.height(4.dp))

        // Android版本
        Text(
            text = androidVersion,
            style = MaterialTheme.typography.bodyMedium
        )
        Spacer(modifier = Modifier.height(4.dp))

        // 内核版本
        Text(
            text = kernelVersion,
            style = MaterialTheme.typography.bodyMedium
        )
    }
}

/**
 * 应用信息卡片组件
 *
 * 显示当前应用的详细信息，包括版本、签名状态和功能配置。
 * 这些信息有助于用户了解应用的当前状态和配置。
 *
 * 显示内容：
 * - 应用版本：当前安装的应用版本
 * - 签名信息：应用签名的哈希值
 * - 签名验证：签名是否有效
 * - 实验功能：是否启用实验性检测
 *
 * @param appVersion 应用版本字符串
 * @param signature 应用签名信息
 * @param signatureValid 签名验证状态
 */
@Composable
fun AppInfoCard(
    appVersion: String,
    signature: String,
    signatureValid: String,
) = Surface(
    modifier = Modifier
        .fillMaxWidth()
        .clip(RoundedCornerShape(16.dp)),
    color = MaterialTheme.colorScheme.surface,
    tonalElevation = 1.dp
) {
    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(20.dp),
        verticalArrangement = Arrangement.Center
    ) {
        // 应用信息标题
        Text(
            text = stringResource(R.string.appinfo),
            style = MaterialTheme.typography.titleSmall,
        )
        Spacer(modifier = Modifier.height(8.dp))

        // 应用版本
        Text(
            text = appVersion,
            style = MaterialTheme.typography.bodyMedium
        )
        Spacer(modifier = Modifier.height(4.dp))

        // 应用签名
        Text(
            text = signature,
            style = MaterialTheme.typography.bodyMedium
        )
        Spacer(modifier = Modifier.height(4.dp))

        // 签名验证状态
        Text(
            text = signatureValid,
            style = MaterialTheme.typography.bodyMedium
        )
        Spacer(modifier = Modifier.height(4.dp))

    }
}

/**
 * 信息卡片容器组件，负责显示系统信息卡片和应用信息卡片
 *
 * 数据源：
 * - deviceInfo：设备型号和制造商信息
 * - androidVersion：Android 系统版本
 * - kernelVersion：Linux 内核版本
 * - appVersion：应用版本号
 * - signature：应用签名哈希
 * - signatureValid：签名验证结果
 *
 * @param viewModel 类型为 MainViewModel ，提供所有必要的信息数据
 *
 * @see MainViewModel 主界面数据和状态管理
 * @see SystemInfoCard 系统信息卡片组件
 * @see AppInfoCard 应用信息卡片组件
 */
@Composable
fun InfoCards(viewModel: MainViewModel) {
    // 响应式收集设备信息
    val deviceInfo by viewModel.deviceInfo.collectAsState()
    val androidVersion by viewModel.androidVersion.collectAsState()
    val kernelVersion by viewModel.kernelVersion.collectAsState()

    // 响应式收集应用信息
    val appVersion by viewModel.appVersion.collectAsState()
    val signature by viewModel.signature.collectAsState()
    val signatureValid by viewModel.signatureValid.collectAsState()

    Column(
        modifier = Modifier.fillMaxSize(),
        verticalArrangement = Arrangement.spacedBy(16.dp)  // 统一的卡片间距
    ) {
        // 系统信息卡片
        SystemInfoCard(
            deviceInfo = deviceInfo,
            androidVersion = androidVersion,
            kernelVersion = kernelVersion
        )

        // 应用信息卡片
        AppInfoCard(
            appVersion = appVersion,
            signature = signature,
            signatureValid = signatureValid,
        )
    }
}

