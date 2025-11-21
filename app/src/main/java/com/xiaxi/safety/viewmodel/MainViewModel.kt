package com.xiaxi.safety.viewmodel

import android.content.Context
import android.content.pm.PackageManager
import android.os.Build
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.xiaxi.safe.util.EnvDetector
import com.xiaxi.safety.R
import com.xiaxi.safety.app.DetectionResult
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch
import java.io.BufferedReader
import java.io.InputStreamReader
import java.security.MessageDigest
import java.security.NoSuchAlgorithmException

/**
 * 主界面ViewModel
 *
 * 数据流设计：
 * ```
 * UI Layer (Compose)
 *        ↓ 用户交互
 * MainViewModel  ← 当前类
 *        ↓ 调用Native方法
 * Native Layer (JNI/C++)
 *        ↓ 系统调用
 * Android System
 * ```
 */
class MainViewModel() : ViewModel() {
    /**
     * 加载状态:当执行耗时的检测任务时,这个状态会被设置为true，UI会显示加载动画或进度条
     *
     * 状态变化时机：
     * - performTask 开始时设置为 true
     * - 检测任务完成或出错时设置为 false
     * - 如果检测结果已缓存，立即设置为 false
     *
     * UI响应：
     * - true: 显示加载指示器，禁用用户交互
     * - false: 隐藏加载指示器，启用用户交互
     *
     * MutableStateFlow("")：创建一个可变的状态流，初始值为""（空字符串）。可在ViewModel内通过 value / update(...) 修改
     * StateFlow<String>：只读视图（接口）并对外暴露它，外部只能订阅状态变化，不能修改值
     */
    private val _isLoading = MutableStateFlow(false)
    val isLoading: StateFlow<Boolean> = _isLoading

    /**
     * 设备信息
     * MutableStateFlow("")：创建一个可变的状态流，初始值为""（空字符串）。可在ViewModel内通过 value / update(...) 修改
     * StateFlow<String>：只读视图（接口）并对外暴露它，外部只能订阅状态变化，不能修改值
     */
    private val _deviceInfo = MutableStateFlow("")
    val deviceInfo: StateFlow<String> = _deviceInfo

    /**
     * Android Version
     * MutableStateFlow("")：创建一个可变的状态流，初始值为""（空字符串）。可在ViewModel内通过 value / update(...) 修改
     * StateFlow<String>：只读视图（接口）并对外暴露它，外部只能订阅状态变化，不能修改值
     */
    private val _androidVersion = MutableStateFlow("")
    val androidVersion: StateFlow<String> = _androidVersion

    /**
     * Linux Kernel Version
     * MutableStateFlow("")：创建一个可变的状态流，初始值为""（空字符串）。可在ViewModel内通过 value / update(...) 修改
     * StateFlow<String>：只读视图（接口）并对外暴露它，外部只能订阅状态变化，不能修改值
     */
    private val _kernelVersion = MutableStateFlow("")
    val kernelVersion: StateFlow<String> = _kernelVersion

    /**
     * App Version
     * MutableStateFlow("")：创建一个可变的状态流，初始值为""（空字符串）。可在ViewModel内通过 value / update(...) 修改
     * StateFlow<String>：只读视图（接口）并对外暴露它，外部只能订阅状态变化，不能修改值
     */
    private val _appVersion = MutableStateFlow("")
    val appVersion: StateFlow<String> = _appVersion

    /**
     * App 签名哈希，对应用签名进行SHA-256哈希计算
     * MutableStateFlow("")：创建一个可变的状态流，初始值为""（空字符串）。可在ViewModel内通过 value / update(...) 修改
     * StateFlow<String>：只读视图（接口）并对外暴露它，外部只能订阅状态变化，不能修改值
     */
    private val _signature = MutableStateFlow("")
    val signature: StateFlow<String> = _signature

    /**
     * 签名验证状态是否有效
     * MutableStateFlow("")：创建一个可变的状态流，初始值为""（空字符串）。可在ViewModel内通过 value / update(...) 修改
     * StateFlow<String>：只读视图（接口）并对外暴露它，外部只能订阅状态变化，不能修改值
     */
    private val _signatureValid = MutableStateFlow("")
    val signatureValid: StateFlow<String> = _signatureValid

    /**
     * 检测结果列表:存储所有检测项的结果数据，其元素为 DetectionResult
     * 数据结构：
     * - List<DetectionResult>: 检测结果的列表
     * - 每个 DetectionResult 包含检测项的完整信息（包含对应检测项的名称、结果和详细信息）
     * - 空列表表示尚未执行检测或检测失败
     */
    private val _detections = MutableStateFlow<List<DetectionResult>>(emptyList())
    val detections: StateFlow<List<DetectionResult>> = _detections

    /**
     * 初始化应用和并收集设备信息，包括如下：
     * - 设备制造商、品牌和型号
     * - Android Version
     * - Linux Kernel Version
     * - App Version
     * - App 签名哈希
     * - 签名验证状态：用 Native 中的 check_certificate_2_V2 函数进行校验签名，判断 apk 是否遭到修改
     *
     * 调用时机：
     * - MainActivity.onCreate()中调用
     * - 应用启动后立即执行，只需要调用一次
     *
     * viewModelScope 是 ViewModel 自带的协程作用域，和 ViewModel 的生命周期绑定
     * launch ：在该作用域里启动一个新的协程（非阻塞），适合发起异步任务：请求数据、读写数据库、计算等
     */
    fun initializeData(context: Context) {
        viewModelScope.launch {
            _deviceInfo.value = String.format(context.getString(R.string.device_information),
                getDevice())
            _androidVersion.value = String.format(context.getString(R.string.android_version),
                Build.VERSION.RELEASE)
            _kernelVersion.value = String.format(context.getString(R.string.kernel_version),
                getKernelVersion())
            _appVersion.value = String.format(context.getString(R.string.app_version),
                getAppVersion(context))
            _signature.value = String.format(context.getString(R.string.app_signatrue),
                getSignature(context))
            if (checkSignature()){
                _signatureValid.value = String.format(context.getString(R.string.app_signatrue_vaild),
                    context.getString(R.string.vaild_true))
            }else{
                _signatureValid.value = String.format(context.getString(R.string.app_signatrue_vaild),
                    context.getString(R.string.vaild_false))
            }
        }
    }

    /**
     * 执行检测任务，执行流程如下：
     * 1. 设置加载状态为true，显示加载指示器
     * 2. 检查是否已有缓存的检测结果
     * 3. 如果有缓存，直接返回并隐藏加载指示器
     * 4. 如果无缓存，执行实际的检测任务
     * 5. 更新检测结果并隐藏加载指示器
     */
    fun performTask(context: Context, packageManager: PackageManager) {
        viewModelScope.launch {
            _isLoading.value = true

            // 检查缓存：如果已有检测结果，直接返回
            if (_detections.value.isNotEmpty()) {
                _isLoading.value = false
                return@launch
            }

            // 开始执行检测任务，getDetections 方法只可以支持非实时性检测项
            val detections = getDetections(context, packageManager)
            _detections.value = detections
            _isLoading.value = false
        }
    }

    /**
     * 获取检测结果
     */
    private suspend fun getDetections(context: Context, packageManager: PackageManager): List<DetectionResult> {
        // 原始Native实现（已注释）：
        // return withContext(Dispatchers.IO) {
        //     val native = Native()
        //     native.getDetections(context, packageManager, enableExperimental).toList()
        // }

        // 当前返回空列表，用于开发测试
        return emptyList()
    }

    private fun getDevice(): String {
        return EnvDetector.showDeviceInfo();
    }

    /**
     * 其他获取的内核版本的方法（如：cat /proc/version），app 的权限都不够，所以当前只能用 uname
     * 需要注意的是 uanme 获取的内核版本并不准确，是可以被修改，如下所示
     * oriole:/ # cat /proc/version
     * Linux version 5.10.228-android13-4-00003-ge99cf82cf6b4-dirty (build-user@build-host) (Android (8508608, based on r450784e) clang version 14.0.7 (https://android.googlesource.com/toolchain/llvm-project 4c603efb0cca074e9238af8b4106c30add4418f6), LLD 14.0.7) #1 SMP PREEMPT Tue Apr 1 12:04:38 UTC 2025
     * oriole:/ # uname -a
     * Linux localhost 5.10.157-android13-4-00001-g5c7ff5dc7aac-ab10381520 #1 SMP PREEMPT Fri Jun 23 18:30:49 UTC 2023 aarch64 Toybox
     */
    private fun getKernelVersion(): String {
        val kernelVersion = StringBuilder()
        try {
            val process = Runtime.getRuntime().exec("uname -r")
            val reader = BufferedReader(InputStreamReader(process.inputStream))
            var line: String?
            while (reader.readLine().also { line = it } != null) {
                kernelVersion.append(line)
            }
            reader.close()
            process.waitFor()
        } catch (e: Exception) {
            e.printStackTrace()
        }
        return kernelVersion.toString().split("-")[0]
    }

    private fun getSignature(context: Context): String {
        return try {
            val pm = context.packageManager
            val packageInfo = pm.getPackageInfo(context.packageName, PackageManager.GET_SIGNATURES)
            val digest = MessageDigest.getInstance("SHA-256")
            for (signature in packageInfo.signatures!!) {
                val signatureBytes = signature.toByteArray()
                digest.update(signatureBytes)
            }
            val hashBytes = digest.digest()
            val hexString = StringBuilder()
            for (hashByte in hashBytes) {
                val hex = Integer.toHexString(0xff and hashByte.toInt())
                if (hex.length == 1) hexString.append('0')
                hexString.append(hex)
            }
            hexString.toString()
        } catch (e: PackageManager.NameNotFoundException) {
            e.printStackTrace()
            "Package name not found."
        } catch (e: NoSuchAlgorithmException) {
            e.printStackTrace()
            "SHA-256 algorithm not found."
        }
    }

    /**
     * 校验 apk 签名是否符合预期
     */
    private fun checkSignature(): Boolean{
        return EnvDetector.checkSignatureV2();
    }

    private fun getAppVersion(context: Context): String {
        return try {
            val pm = context.packageManager
            val packageInfo = pm.getPackageInfo(context.packageName, 0)
            packageInfo.versionName ?: "Unknown"
        } catch (e: PackageManager.NameNotFoundException) {
            e.printStackTrace()
            "Unknown"
        }
    }
}