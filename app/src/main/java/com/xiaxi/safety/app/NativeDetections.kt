package com.xiaxi.safety.app

import android.content.Context
import android.content.pm.PackageManager

/**
 * 在 com.reveny.nativecheck.viewmodel.MainViewModel 中调用 NativeDetections 类中的 getDetections 方法
 */
class NativeDetections {

    /**
     * @return 该 DetectionResult 数组，包含所有检测到的异常项目
     * 数组为空表示未检测到当前测试机环境相关异常，其中每个 DetectionResult 都包含检测项目的名称和详细描述
     */
    external fun getDetections(context: Context, pm: PackageManager): Array<DetectionResult>
}