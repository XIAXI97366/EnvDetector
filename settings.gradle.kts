pluginManagement {
    repositories {
        google {
            content {
                includeGroupByRegex("com\\.android.*")
                includeGroupByRegex("com\\.google.*")
                includeGroupByRegex("androidx.*")
            }
        }
        mavenCentral()
        // pluginManagement 块用于配置插件的管理和解析
        // 它定义了 Gradle 在解析插件时应该使用的仓库。
        // 在这个块中，maven("https://jitpack.io") 表示插件的解析过程中，Gradle 会从 JitPack 仓库中查找插件。
        maven("https://jitpack.io")
        gradlePluginPortal()
    }
}

dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.PREFER_SETTINGS)
    repositories {
        google()
        mavenCentral()
        // dependencyResolutionManagement 块用于配置项目依赖的解析。
        // 它定义了 Gradle 在解析项目依赖时应该使用的仓库。
        // 在这个块中，maven("https://jitpack.io") 表示在解析项目依赖时，Gradle 会从 JitPack 仓库中查找依赖。
        maven("https://jitpack.io")
    }
}

rootProject.name = "Safety"
include(":app")
include(":xiaxiSafe")
include(":stub")
