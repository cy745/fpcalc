# Chromaprint fpcalc using MediaCodec for Android platform

## 简介

针对Android平台的一个基于Chromaprint的音频指纹计算库，可计算音频指纹用于判断音频是否“相同”，可用音频指纹调用 [MusicBrainz](https://musicbrainz.org/) 的API查询音频的相关信息。

本仓库的核心代码基于Chromaprint的`fpcalc.cpp`。与原始实现不同，使用Android NDK提供的`MediaCodec`来替代`FFMpeg`的`AvCodec`进行音频解码。降低了动态库的大小。同时针对Android平台，提供了使用`FileDescriptor`读取文件的方法，以便更好地集成到Android应用中。

## 功能与特性

- 使用`MediaCodec`替代`FFMpeg`的`AvCodec`进行音频解码，降低动态库大小。
- 提供使用`FileDescriptor`读取文件的方法，方便Android应用集成。

## 参考与引用

- Chromaprint官方仓库: [https://github.com/acoustid/chromaprint](https://github.com/acoustid/chromaprint)
- fpcalc-android项目: [https://github.com/QuickLyric/fpcalc-android](https://github.com/QuickLyric/fpcalc-android)
- ffmpeg-android-maker项目: [https://github.com/Javernaut/ffmpeg-android-maker](https://github.com/Javernaut/ffmpeg-android-maker)

## 如何使用

```gradle
// 导入jitpack源
maven(url = "https://jitpack.io")

// 所需使用的project中引入library
implementation("com.github.cy745:fpcalc:<version>")
```

```kotlin
// 配置传入fpcalc的参数，fd为文件的FileDescriptor
val args: Array<String> = arrayOf("-json", "$fd")

// 调用Fpcalc
val result: String = Fpcalc.calc(args)
```

result的格式根据传入参数可能不同，需要注意，以下是JSON格式的示例
```json
{
  "duration": 42.62,
  "fingerprint": "AQABN0myKWmYBkJwvEfDo-gcHWGeQ0eYo8N__GjeoT_y5-jh_eiP_PjR48djvEc-G_6gXcHT4McDfUeYvWiPozeabkEO8QryhMrx40c_ozmB5kF5tDHCfDku3NAO0-hz6Md_9EoD68MHP8V_Qz_-oEf-4ymOEp8uXMIdoXmxC8d7tDsS5jHSH7UY_Dt6o_mCbpuH5vjxGTfR50hz-ERn4SX6HFYOHzlhBWGJQ8tn9AzCfPHQcsdn9EZKDoco32jW4B1KQjzyD5aIw8cP__iPVFvRozbE5-iDdonQnMYnhGVwhC-H5_iORg_0zuiO4zy65_CHH-IP60c_HK0FLc_Ro3mO44VP6OiPH9fxQzx-pCcO67hIHH7xwd1RPuiPKWcO5kb67Jh0RdCXoDdikTm-FI10XMyJIyeJN2OgfTquCYcuGDm-4ceRC-ehHn-D4zP84oeI6oPe4T8uI_qD52gfnNKA-Pjg44RxHpR8_PAPlTjSE_8RHN3xQyV-vEcv-BNu-DMu4oj2Cz2DyoH1oM-OSpQMvyhv_MiP0mjiLC7-F_mN5j2q7UEPPxOcz3h_JMuJL0d-ouGYwmoQ6tDVICSzw-dxYsdRyziu4zeebMcnNP_QA_mhKiR6Huk9dNVw_MibQeuQ80T34TKa_EfxEnk_9EZzvC9uHqVepIf7Fz16_Dhw-IcVH8cF66gBRyBgBipimHBGEUIEwkQYIp0EhmhDLBCGOCIUIYAoJ4QgTgwilBCIEKIUJAY4Ax0yAChCiAWAUGkcEUqJYpFBAACnhHLGAKEEEAIKxawQgEBliRAAWCGMQEYABpQCzBJBHCCAQEMEEcIIZRChgDBAxBWcKGEcE0YIKywQDhglCARAASaIQM4BghQRBBupIFAiOKSIFFtihpAgDhjCmGPYGoeAskwYy4hCgDDLDDEGMIIQAoIIAoQUhAhChFDCMGCEAYQBhzxxgDhjoXREEMCAMcIJgJyEQhggARGECIaMAA4AIBQSRDxCiBMG"
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

- 解决各种闪退问题
- 完善API，降低Android平台使用的难度
- 完全剔除对libavutils的依赖（目前还有resample部分依赖libavutils用于缓冲区的创建）
- ......

## 贡献与反馈

欢迎任何形式的贡献和反馈。如果你在使用过程中遇到问题，或者有任何建议，请随时提交issue或pull request。

