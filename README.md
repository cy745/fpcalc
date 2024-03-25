# Chromaprint fpcalc using MediaCodec for Android platform

## 简介

针对Android平台的一个基于Chromaprint的音频指纹计算库，可计算音频指纹用于判断音频是否“相同”，可用音频指纹调用 [Acoustid](https://acoustid.org/webservice) 的API查询 [MusicBrainz](https://musicbrainz.org/) 库中所存有的与该音频相似的数据的相关信息。

本仓库的核心代码基于Chromaprint的`fpcalc.cpp`。与原始实现不同，使用Android NDK提供的`MediaCodec`来替代`FFMpeg`的`AvCodec`进行音频解码。降低了动态库的大小。同时针对Android平台，提供了使用`FileDescriptor`读取文件的方法，以便更好地集成到Android应用中。

## 功能与特性

- 使用`MediaCodec`替代`FFMpeg`的`AvCodec`进行音频解码，降低动态库大小。
- 提供使用`FileDescriptor`读取文件的方法，方便Android应用集成。

## 参考与引用

- Chromaprint官方仓库: [https://github.com/acoustid/chromaprint](https://github.com/acoustid/chromaprint)
- fpcalc-android项目: [https://github.com/QuickLyric/fpcalc-android](https://github.com/QuickLyric/fpcalc-android)
- ffmpeg-android-maker项目: [https://github.com/Javernaut/ffmpeg-android-maker](https://github.com/Javernaut/ffmpeg-android-maker)

## 如何使用

[![](https://jitpack.io/v/cy745/fpcalc.svg)](https://jitpack.io/#cy745/fpcalc)

```gradle
// 导入jitpack源
maven(url = "https://jitpack.io")

// 所需使用的project中引入library 
implementation("com.github.cy745:fpcalc:<version>")
```

```kotlin
// 配置传入fpcalc的参数，fd为文件的FileDescriptor
val params: FpcalcParams = FpcalcParams(
  targetFd = fd    // Int
)

// 或者传入文件路径
val params: FpcalcParams = FpcalcParams(
  targetFilePath = filePath    // String
)

// 调用Fpcalc
val result: FpcalcResult = Fpcalc.calc(params)

// 输入的FpcalcParams定义如下
data class FpcalcParams(
    val targetFd: Int = -1,
    val targetFilePath: String? = null,
    val gMaxDuration: Int = 120,    // in second
    val gRaw: Boolean = false,
    val gSigned: Boolean = false,
    val gAlgorithm: Int = 2,
)

// 输出的FpcalcResult定义如下
class FpcalcResult {
    var fingerprint: String? = null
    var rawFingerprint: String? = null
    var errorMessage: String? = null
    var sourceDurationMs: Long = 0
    var sourceSampleRate: Int = 0
    var sourceChannels: Int = 0
    var sourceLength: Int = 0
}
```

## 参与开发

### 构建项目

```bash
# 拉取项目，注意要拉取子项目
git clone --recursive https://github.com/cy745/fpcalc

# 若只拉取了父项目，没加--recursive参数的话，进入项目目录后执行下面的命令即可
git submodule update --init --recursive
```

### 后续任务

- ✅ 解决各种闪退问题 
- ✅ 完善API，降低Android平台使用的难度
- 🔘 完全剔除对libavutils的依赖（目前还有resample部分依赖libavutils用于缓冲区的创建）
- ......

## 贡献与反馈

欢迎任何形式的贡献和反馈。如果你在使用过程中遇到问题，或者有任何建议，请随时提交issue或pull request。

