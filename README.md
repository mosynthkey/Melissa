<h1 align="center">A music player for musical instrument practice<br>for macOS and Windows</h1>

![uiimage](docs/images/screenshot.png)

<div align="center"> <a href="https://mosynthkey.github.io/Melissa/">Official site (Japanese)</a></div>

## Downloads
### macOS
- [Version 2.5.0](https://github.com/mosynthkey/Melissa/releases/download/v2.5.0/Melissa_2_5_0.dmg)

### Windows
- [Version 2.5.0 (64bit)](https://github.com/mosynthkey/Melissa/releases/download/v2.5.0/Melissa_2.5.0_64.zip)
- [Version 2.5.0 (32bit)](https://github.com/mosynthkey/Melissa/releases/download/v2.5.0/Melissa_2.5.0_32.zip)

## Manual
- [English](https://github.com/mosynthkey/Melissa/wiki/Manual-(English))
- [日本語](https://github.com/mosynthkey/Melissa/wiki/Manual-(Japanese))

## How to build
1. Get [Projucer](https://juce.com/discover/projucer).
2. Install submodules
```
git submodule update --init
```
3. Open [Melissa/Melissa.jucer](Melissa/Melissa.jucer) with Projucer
4. Create and open a Xcode project file or Visual Studio solution on Projucer

To build on Windows, please get ASIO driver or disable ASIO from Projucer.
See [this](ThirdParty/asio/how%20to%20get%20asio%20sdk.md) for the detail.

## Contact
[Twitter](https://twitter.com/Melissa__Player)

## Libraries
- [JUCE](https://juce.com) 
- [SoundTouch](https://www.surina.net/soundtouch/)

## License
[LGPL-2.1 (c) 2021 Masaki Ono](LICENSE)
