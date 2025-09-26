plugins {
    alias(libs.plugins.android.application)
    alias(libs.plugins.kotlin.android)
}

android {
    namespace = "com.xiaxi.safety"
    compileSdk = 36

    // 开启 V1 V2 V3 V4 签名
    signingConfigs {
        create("release") {
            // storeFile = file("C:\\Users\\97366\\xiaxi97366.jks")
            storeFile = file("/Users/xiaxi/xiaxi_workspace/tools/AS_Signtrues/xiaxi97366.jks")
            storePassword = "973668008"
            keyAlias = "key0"
            keyPassword = "973668008"

            enableV1Signing = true
            enableV2Signing = true
            enableV3Signing = true
            enableV4Signing = true
        }
    }

    defaultConfig {
        applicationId = "com.xiaxi.safety"
        minSdk = 23
        targetSdk = 34
        versionCode = 1
        versionName = "1.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"

        // ndkVersion = "26.2.11394342"
        // ndkPath = "E:\\Environment_Variable\\android-ndk-r26c-windows\\android-ndk-r26c"

        // ndkVersion = "27.2.12479018"
        // ndkPath = "D:\\NDK\\android-ndk-r27c-windows\\android-ndk-r27c"

        ndkVersion = "28.2.13676358"
        ndkPath = "/opt/homebrew/share/android-ndk"

        externalNativeBuild {
            cmake {
                cppFlags("-std=c++11")
            }
        }
    }

    buildTypes {
        release {
            signingConfig = signingConfigs.getByName("release")
            isMinifyEnabled = false
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }
    buildFeatures {
        viewBinding = true
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    packaging {
        resources {
            // 直接排除冲突文件
            excludes += "META-INF/versions/9/OSGI-INF/MANIFEST.MF"
            // 如仍有类似冲突，可放宽：
            // excludes += "META-INF/versions/**"
        }
    }
    kotlinOptions {
        jvmTarget = "17"
    }
}

dependencies {
    implementation(project(":xiaxiSafe"))

    implementation(libs.appcompat)
    implementation(libs.material)
    implementation(libs.constraintlayout)
    implementation(libs.androidx.core.ktx)
    testImplementation(libs.junit)
    androidTestImplementation(libs.ext.junit)
    androidTestImplementation(libs.espresso.core)
}