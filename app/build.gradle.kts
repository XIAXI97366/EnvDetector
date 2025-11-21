plugins {
    alias(libs.plugins.android.application)
    alias(libs.plugins.kotlin.android)
    ///// UI 新增 /////
    alias(libs.plugins.kotlin.compose)
    alias(libs.plugins.kotlin.serialization)
    alias(libs.plugins.hilt)
    alias(libs.plugins.ksp)
    ///// UI 新增 /////
}

android {
    namespace = "com.xiaxi.safety"
    compileSdk = 36

    // 开启 V1 V2 V3 V4 签名
    signingConfigs {
        create("release") {
            // storeFile = file("C:\\Users\\97366\\xiaxi97366.jks")
            storeFile = file("/Users/xiaxi/xiaxi/xiaxi97366.jks")
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
        minSdk = 27
        targetSdk = 34
        versionCode = 100
        versionName = "1.0.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"

        ndkVersion = "29.0.14206865"
        // ndkPath = "E:\\Environment_Variable\\android-ndk-r29-windows\\android-ndk-r29"
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

    ///// UI 新增  📦 使用 BOM 管理 Compose 版本 /////
    implementation(platform(libs.androidx.compose.bom))

    ///// UI 新增  🎨 Compose UI组件 /////
    implementation(libs.bundles.compose)

    ///// UI 新增  📱 AndroidX核心 /////
    implementation(libs.bundles.androidx.core)

    ///// UI 新增  🔄 生命周期组件 /////
    implementation(libs.bundles.lifecycle)

    ///// UI 新增  🔧 依赖注入 (Hilt) /////
    implementation(libs.bundles.hilt)
    ksp(libs.hilt.compiler)

    ///// UI 新增  💾 数据存储 /////
    implementation(libs.bundles.datastore)

    ///// UI 新增  🎨 UI和主题 /////
    implementation(libs.bundles.ui.theme)

    implementation(libs.bundles.core)

    implementation(libs.bundles.accompanist)

    ///// UI 新增  🧪 测试依赖 /////
    testImplementation("junit:junit:4.13.2")
    androidTestImplementation("androidx.test.ext:junit:1.2.1")
    androidTestImplementation("androidx.test.espresso:espresso-core:3.6.1")
    androidTestImplementation(platform(libs.androidx.compose.bom))
    androidTestImplementation("androidx.compose.ui:ui-test-junit4")
    debugImplementation("androidx.compose.ui:ui-tooling")
    debugImplementation("androidx.compose.ui:ui-test-manifest")
}