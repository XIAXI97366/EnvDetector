// Top-level build file where you can add configuration options common to all sub-projects/modules.



plugins {
    alias(libs.plugins.android.application) apply false
    alias(libs.plugins.android.library) apply false
}


extra.apply {
    set("compileSdk", 34)
    set("minSdk", 21)
    set("targetSdk", 34)
    set("ndkVersion", "27.2.12479018")
    set("cmakeVersion", "3.22.1")
}