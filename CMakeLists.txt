cmake_minimum_required(VERSION 3.22)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "13.5" CACHE STRING "Minimum macOS deployment version")
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build universal binary")
endif()

set(MELISSA_VERSION "4.0.1")
set(MELISSA_VERSION_HEX "0x40001")

project(Melissa VERSION ${MELISSA_VERSION})

# Set MSVC runtime library consistently
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

include(FetchContent)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Submodule/JUCE)

set(JUCE_BUILD_HELPER_TOOLS OFF CACHE BOOL "")
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "")
set(JUCE_BUILD_TESTS OFF CACHE BOOL "")

set(BUILD_TESTING OFF CACHE BOOL "")
set(BUILD_EXAMPLES OFF CACHE BOOL "")

juce_add_gui_app(Melissa
    PRODUCT_NAME "Melissa"
    VERSION ${MELISSA_VERSION}
    COMPANY_NAME "Melissa Audio"
    COMPANY_COPYRIGHT "Copyright(c) 2025 Masaki Ono"
    BUNDLE_ID "com.melissa-audio.melissa"
    ICON_BIG "Resource/icon.png"
    BLUETOOTH_PERMISSION_ENABLED TRUE
    BLUETOOTH_PERMISSION_TEXT "For Bluetooth MIDI"
    DOCUMENT_BROWSER_ENABLED TRUE
)

set_target_properties(Melissa PROPERTIES
    JUCE_COMPANY_NAME "Melissa Audio"
    JUCE_COMPANY_COPYRIGHT "Copyright(c) 2025 Masaki Ono"
    JUCE_COMPANY_WEBSITE ""
    JUCE_COMPANY_EMAIL ""
    JUCE_VERSION "${MELISSA_VERSION}"
    JUCE_VERSION_HEX "${MELISSA_VERSION_HEX}"
)

juce_generate_juce_header(Melissa)

target_sources(Melissa PRIVATE
    Source/Audio/MelissaAudioEngine.cpp
    Source/Audio/MelissaBeepGenerator.cpp
    Source/Audio/MelissaBPMDetector.cpp
    Source/Audio/MelissaMetronome.cpp
    Source/Audio/soundtouch/BPMDetect_for_Melissa.cpp
    Source/Main.cpp
    Source/MainComponent.cpp
    Source/MelissaCommand.cpp
    Source/MelissaDataSource.cpp
    Source/MelissaDataSource.h
    Source/MelissaDefinitions.cpp
    Source/MelissaExporter.cpp
    Source/MelissaExportManager.cpp
    Source/MelissaModel.cpp
    Source/MelissaShortcutManager.cpp
    Source/MelissaStemProvider.cpp
    Source/MelissaStemProvider.h
    Source/MelissaUpdateChecker.cpp
    Source/UI/MelissaAboutComponent.h
    Source/UI/MelissaBPMSettingComponent.h
    Source/UI/MelissaBrowserComponent.cpp
    Source/UI/MelissaBrowserComponent.h
    Source/UI/MelissaButtons.h
    Source/UI/MelissaCommandComboBox.cpp
    Source/UI/MelissaCommandComboBox.h
    Source/UI/MelissaDoubleClickEditLabel.h
    Source/UI/MelissaExportComponent.cpp
    Source/UI/MelissaExportComponent.h
    Source/UI/MelissaFileListBox.h
    Source/UI/MelissaIncDecButton.h
    Source/UI/MelissaInputDialog.h
    Source/UI/MelissaLabel.h
    Source/UI/MelissaLookAndFeel.h
    Source/UI/MelissaLoopRangeComponent.cpp
    Source/UI/MelissaLoopRangeComponent.h
    Source/UI/MelissaMarkerListBox.cpp
    Source/UI/MelissaMarkerListBox.h
    Source/UI/MelissaMarkerListener.h
    Source/UI/MelissaMarkerMemoComponent.cpp
    Source/UI/MelissaMarkerMemoComponent.h
    Source/UI/MelissaModalDialog.cpp
    Source/UI/MelissaModalDialog.h
    Source/UI/MelissaOptionDialog.h
    Source/UI/MelissaPlaylistComponent.cpp
    Source/UI/MelissaPlaylistComponent.h
    Source/UI/MelissaPopupMessageComponent.cpp
    Source/UI/MelissaPopupMessageComponent.h
    Source/UI/MelissaPracticeTableListBox.cpp
    Source/UI/MelissaPracticeTableListBox.h
    Source/UI/MelissaPreCountSettingComponent.h
    Source/UI/MelissaProgressBarComponent.h
    Source/UI/MelissaScrollLabel.h
    Source/UI/MelissaSectionComponent.cpp
    Source/UI/MelissaSectionComponent.h
    Source/UI/MelissaShortcutComponent.cpp
    Source/UI/MelissaShortcutComponent.h
    Source/UI/MelissaSimpleTextButton.h
    Source/UI/MelissaSpeedTrainingProgressComponent.cpp
    Source/UI/MelissaSpeedTrainingProgressComponent.h
    Source/UI/MelissaStemControlComponent.cpp
    Source/UI/MelissaStemControlComponent.h
    Source/UI/MelissaStemSeparationSelectComponent.cpp
    Source/UI/MelissaStemSeparationSelectComponent.h
    Source/UI/MelissaTapTempoButton.cpp
    Source/UI/MelissaTapTempoButton.h
    Source/UI/MelissaTextEditorWithClearButton.cpp
    Source/UI/MelissaTextEditorWithClearButton.h
    Source/UI/MelissaTutorialComponent.cpp
    Source/UI/MelissaTutorialComponent.h
    Source/UI/MelissaUISettings.h
    Source/UI/MelissaWaveformControlComponent.cpp
    Source/UI/MelissaWaveformControlComponent.h
    Source/UI/MelissaWaveformMouseEventComponent.cpp
    Source/UI/MelissaWaveformMouseEventComponent.h
    Submodule/soundtouch/source/SoundTouch/AAFilter.cpp
    Submodule/soundtouch/source/SoundTouch/cpu_detect_x86.cpp
    Submodule/soundtouch/source/SoundTouch/FIFOSampleBuffer.cpp
    Submodule/soundtouch/source/SoundTouch/FIRFilter.cpp
    Submodule/soundtouch/source/SoundTouch/InterpolateCubic.cpp
    Submodule/soundtouch/source/SoundTouch/InterpolateLinear.cpp
    Submodule/soundtouch/source/SoundTouch/InterpolateShannon.cpp
    Submodule/soundtouch/source/SoundTouch/mmx_optimized.cpp
    Submodule/soundtouch/source/SoundTouch/PeakFinder.cpp
    Submodule/soundtouch/source/SoundTouch/RateTransposer.cpp
    Submodule/soundtouch/source/SoundTouch/SoundTouch.cpp
    Submodule/soundtouch/source/SoundTouch/sse_optimized.cpp
    Submodule/soundtouch/source/SoundTouch/TDStretch.cpp
    ThirdParty/spleet/input_file.cpp
    ThirdParty/spleet/io.cpp
    ThirdParty/spleet/output_folder.cpp
    ThirdParty/spleet/split.cpp
    ThirdParty/spleet/utils.cpp
    ThirdParty/demucs.onnx/src/model_apply.cpp
    ThirdParty/demucs.onnx/src/model_inference.cpp
    ThirdParty/demucs.onnx/src/dsp.cpp
)

target_include_directories(Melissa PRIVATE
    Submodule/soundtouch/include
    Submodule/soundtouch/source/SoundTouch
    Submodule/eigen
    Submodule/json/single_include
    Submodule/signalsmith-stretch
    Submodule/signalsmith-stretch/dsp
    ThirdParty/spleet
    ThirdParty/spleeterpp/include
    ThirdParty/libtensorflow-cpu-darwin-universal-binary-2.8.0/include
    Source
    Source/Audio
    Source/Audio/soundtouch
    Source/UI
    ThirdParty/libonnxruntime/include
    ThirdParty/demucs.onnx/src
)

target_compile_definitions(Melissa PRIVATE
    ONNX_ML=1
    ONNX_NAMESPACE=onnx
    _USE_MATH_DEFINES
    MELISSA_FULL_VERSION
    JUCE_SILENCE_XCODE_15_LINKER_WARNING
    JUCE_USE_WIN_WEBVIEW2
    MELISSA_USE_STEM_SEPARATION
    ENABLE_SPEED_TRAINING
)

target_link_libraries(Melissa PRIVATE
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_cryptography
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    spleeter
    spleeter_common
    tensorflow
)

if(APPLE)
    target_link_libraries(Melissa PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libonnxruntime/libs/macos-arm64_x86_64/libonnxruntime.1.19.2.dylib
    )
elseif(WIN32)
    target_link_libraries(Melissa PRIVATE
        onnxruntime
    )
endif()

juce_add_binary_data(MelissaBinaryData
    SOURCES
    Resource/icon.png
    Resource/Icon/melissa_header.svg
    Resource/Icon/add.svg
    Resource/Icon/arrow_left.svg
    Resource/Icon/arrow_right.svg
    Resource/Icon/clear.svg
    Resource/Icon/detail.svg
    Resource/Icon/down.svg
    Resource/Icon/dummy_icon.svg
    Resource/Icon/export_current.svg
    Resource/Icon/export_playlist_practicelist.svg
    Resource/Icon/export_playlist.svg
    Resource/Icon/export_practiceList.svg
    Resource/Icon/export.svg
    Resource/Icon/loop_onesong.svg
    Resource/Icon/loop_playlist.svg
    Resource/Icon/next_button.svg
    Resource/Icon/playlist_add_file.svg
    Resource/Icon/playlist_add_playing.svg
    Resource/Icon/playlist_add.svg
    Resource/Icon/playlist_edit.svg
    Resource/Icon/playlist_remove.svg
    Resource/Icon/prev_button.svg
    Resource/Icon/select.svg
    Resource/Icon/speaker.svg
    Resource/Icon/trim.svg
    Resource/Icon/up.svg
    Resource/Icon/left.svg
    Resource/Icon/right.svg
    Resource/Icon/home.svg
    Resource/Icon/go.svg
    Resource/Icon/play_button.svg
    Resource/Language/en-US.txt
    Resource/Language/ja-JP.txt
    Resource/logo.png
)

target_link_libraries(Melissa PRIVATE MelissaBinaryData)

# Ensure consistent runtime library usage
if(MSVC)
    set_target_properties(Melissa PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
endif()

if (APPLE)
    set_target_properties(Melissa PROPERTIES
        MACOSX_BUNDLE TRUE
        XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Manual"
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "-"
        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ""
        XCODE_ATTRIBUTE_PROVISIONING_PROFILE_SPECIFIER ""
        XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO"
        XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
        XCODE_EMBED_FRAMEWORKS "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libtensorflow-cpu-darwin-universal-binary-2.8.0/lib/libtensorflow_framework.2.dylib;${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libtensorflow-cpu-darwin-universal-binary-2.8.0/lib/libtensorflow.2.dylib;${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libonnxruntime/libs/macos-arm64_x86_64/libonnxruntime.1.19.2.dylib"
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "@executable_path/../Frameworks"
    )

    add_custom_command(TARGET Melissa POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_BUNDLE_CONTENT_DIR:Melissa>/Frameworks"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libtensorflow-cpu-darwin-universal-binary-2.8.0/lib/libtensorflow.2.dylib"
            "$<TARGET_BUNDLE_CONTENT_DIR:Melissa>/Frameworks/"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libtensorflow-cpu-darwin-universal-binary-2.8.0/lib/libtensorflow_framework.2.dylib"
            "$<TARGET_BUNDLE_CONTENT_DIR:Melissa>/Frameworks/"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libonnxruntime/libs/macos-arm64_x86_64/libonnxruntime.1.19.2.dylib"
            "$<TARGET_BUNDLE_CONTENT_DIR:Melissa>/Frameworks/"
    )
endif()

target_link_options(Melissa PRIVATE
    -L${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/spleeterpp/lib
    -L${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libtensorflow-cpu-darwin-universal-binary-2.8.0/lib
    -L${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty
)

if(WIN32)
    target_link_directories(Melissa PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/spleeterpp/lib/Windows/$<CONFIG>
        ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libtensorflow-cpu-windows-x86_64-2.8.0/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libonnxruntime/libs/win-x86_64
    )

    # Copy ONNX Runtime DLL to output directory post-build
    add_custom_command(TARGET Melissa POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libonnxruntime/libs/win-x86_64/onnxruntime.dll"
            $<TARGET_FILE_DIR:Melissa>
    )
    
    # Copy TensorFlow CPU DLL to output directory post-build
    add_custom_command(TARGET Melissa POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libtensorflow-cpu-windows-x86_64-2.8.0/lib/tensorflow.dll"
            $<TARGET_FILE_DIR:Melissa>
    )
endif()
