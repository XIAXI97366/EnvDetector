package com.xiaxi.safety

import android.R
import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.WindowCompat
import androidx.core.view.WindowInsetsCompat
import androidx.core.view.WindowInsetsControllerCompat
import com.xiaxi.safety.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {
    private var binding: ActivityMainBinding? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // 设置沉浸式全屏模式
        setupImmersiveMode()

        // 隐藏 ActionBar
        if (getSupportActionBar() != null) {
            getSupportActionBar()!!.hide()
        }

        binding = ActivityMainBinding.inflate(getLayoutInflater())
        setContentView(binding!!.getRoot())

        //if(new File("/data/data/com.topjohnwu.magisk").exists()) {}

        // Example of a call to a native method
    }

    private fun setupImmersiveMode() {
        // 使用新的WindowInsetsController API
        WindowCompat.setDecorFitsSystemWindows(getWindow(), false)

        val windowInsetsController =
            WindowCompat.getInsetsController(getWindow(), getWindow().getDecorView())

        if (windowInsetsController != null) {
            // 隐藏状态栏和导航栏
            windowInsetsController.hide(WindowInsetsCompat.Type.systemBars())
            // 设置沉浸式模式行为
            windowInsetsController.setSystemBarsBehavior(
                WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
            )
        }


        // 设置状态栏和导航栏颜色为白色
        getWindow().setStatusBarColor(getResources().getColor(R.color.white))
        getWindow().setNavigationBarColor(getResources().getColor(R.color.white))


        // 设置状态栏文字为深色
        getWindow().getDecorView().setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR or View.SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR
        )
    }


    /**
     * A native method that is implemented by the 'checkrom' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String?

    companion object {
        // Used to load the 'checkrom' library on application startup.
        init {
            System.loadLibrary("checkrom")
        }
    }
}