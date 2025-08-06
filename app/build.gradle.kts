plugins {
    alias(libs.plugins.android.application)
}

android {
    namespace = "com.example.checkrom"
    compileSdk = 35

    // 开启 V1 V2 V3 V4 签名
    signingConfigs {
        create("release") {
            storeFile = file("C:\\Users\\XIAXI\\xiaxi97366.jks")
            storePassword = "xiaxi973668008"
            keyAlias = "key0"
            keyPassword = "xiaxi97366"

            enableV1Signing = true
            enableV2Signing = true
            enableV3Signing = true
            enableV4Signing = true
        }
    }

    defaultConfig {
        applicationId = "com.example.checkrom"
        minSdk = 23
        targetSdk = 34
        versionCode = 1
        versionName = "1.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"

         // ndkVersion = "26.2.11394342"
         // ndkPath = "E:\\Environment_Variable\\android-ndk-r26c-windows\\android-ndk-r26c"

        ndkVersion = "27.2.12479018"
        ndkPath = "D:\\NDK\\android-ndk-r27c-windows\\android-ndk-r27c"

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
        sourceCompatibility = JavaVersion.VERSION_21
        targetCompatibility = JavaVersion.VERSION_21
    }
}

dependencies {
    implementation(project(":xiaxiSafe"))

    implementation(libs.appcompat)
    implementation(libs.material)
    implementation(libs.constraintlayout)
    testImplementation(libs.junit)
    androidTestImplementation(libs.ext.junit)
    androidTestImplementation(libs.espresso.core)
}