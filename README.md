<h1 align="center">A music player for musical instrument practice<br>for macOS and Windows</h1>

![uiimage](docs/images/screenshot.png)


<div align="center"> Official site
<a href="https://mosynthkey.github.io/Melissa/index_en.html">English</a> / <a href="https://mosynthkey.github.io/Melissa/index.html">日本語</a></div>

## Downloads
### macOS
- [Version 4.0.0](https://github.com/mosynthkey/Melissa/releases/download/v4.0.0/Melissa_4_0_0.dmg)

### Windows
- [Version 4.0.0 (64bit)](https://github.com/mosynthkey/Melissa/releases/download/v4.0.0/Melissa_4.0.0_64.zip)

## Manual
- [English](https://github.com/mosynthkey/Melissa/wiki/Manual-(English))
- [日本語](https://github.com/mosynthkey/Melissa/wiki/Manual-(Japanese))

## How to build
1. Get CMake
2. Install thirdparty libraries
Download and extract libtensorflow 2.8.0 to [ThirdParty](ThirdParty)
- [macOS](https://github.com/mosynthkey/libtensorflow-cpu-darwin-universal-binary/releases/tag/v2.8.0)
- [Windows](https://storage.googleapis.com/tensorflow/libtensorflow/libtensorflow%2Dcpu%2Dwindows%2Dx86_64%2D2.8.0.zip)

Download and extract libonnxruntime to [ThirdParty](ThirdParty)
- https://github.com/mosynthkey/Melissa/releases/download/v4.0.0/libonnxruntime.zip

3. Install submodules
```
git submodule update --init
```
4. Create project with CMake
```
mkdir build
cmake -S . -B build -DCMAKE_POLICY_VERSION_MINIMUM="3.5"
```

5. Open and build with the project
On Windows, please install "WebView" package using Nuget Package Manager

To build on Windows, please get ASIO driver or disable ASIO from Projucer.
See [this](ThirdParty/asio/how%20to%20get%20asio%20sdk.md) for the detail.

## Contact
[Twitter](https://x.com/Melissa__Player)

## Libraries
- [JUCE](https://juce.com) 
- [SoundTouch](https://www.surina.net/soundtouch/)
- [spleeterpp](https://github.com/gvne/spleeterpp)
- [speet](https://github.com/gvne/spleet)
- [nlohmann/json](https://github.com/nlohmann/json)
- [TensorFlow for C](https://www.tensorflow.org/install/)　
- [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page)
- [demucs.onnx](https://github.com/sevagh/demucs.onnx)

## License
[LGPL-2.1 (c) 2024 Masaki Ono](LICENSE)
