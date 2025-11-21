package com.xiaxi.safety

import android.content.Context
import com.xx.shell.RiskCheckApplication
import dagger.hilt.android.HiltAndroidApp

@HiltAndroidApp
class App : RiskCheckApplication() {
    override fun attachBaseContext(base: Context) {
        super.attachBaseContext(base)
    }

    override fun onCreate() {
        super.onCreate()
    }
}