cmake_minimum_required(VERSION 3.22)

project(Melissa VERSION 3.2.0)

include(FetchContent)

set(USE_OPENBLAS ON CACHE BOOL "Use OpenBLAS")
set(BLA_VENDOR OpenBLAS)
if(APPLE)
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/opt/homebrew/opt/openblas")
    find_package(BLAS REQUIRED)
elseif(WIN32)
    set(OPENBLAS_VERSION "0.3.29")
    set(OPENBLAS_URL "https://github.com/OpenMathLib/OpenBLAS/releases/download/v${OPENBLAS_VERSION}/OpenBLAS-${OPENBLAS_VERSION}_x64.zip")
    
    FetchContent_Declare(
        openblas
        URL ${OPENBLAS_URL}
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(openblas)
    
    set(OPENBLAS_ROOT "${openblas_SOURCE_DIR}")
    set(OPENBLAS_INCLUDE_DIR "${OPENBLAS_ROOT}/include")
    set(OPENBLAS_LIBRARIES "${OPENBLAS_ROOT}/lib/libopenblas.dll.a")
    set(BLAS_LIBRARIES "${OPENBLAS_LIBRARIES}" CACHE PATH "Path to BLAS library")
    set(BLAS_FOUND TRUE CACHE BOOL "BLAS found")
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Submodule/JUCE)

set(JUCE_BUILD_HELPER_TOOLS OFF CACHE BOOL "")
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "")
set(JUCE_BUILD_TESTS OFF CACHE BOOL "")

set(BUILD_TESTING OFF CACHE BOOL "")
set(BUILD_EXAMPLES OFF CACHE BOOL "")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Submodule/demucs.cpp EXCLUDE_FROM_ALL)

juce_add_gui_app(Melissa
    PRODUCT_NAME "Melissa"
    VERSION "3.2.0"
    COMPANY_NAME "Melissa Audio"
    COMPANY_COPYRIGHT "Copyright(c) 2025 Masaki Ono"
    BUNDLE_ID "com.melissa-audio.melissa"
    ICON_BIG "Resource/icon.png"
    BLUETOOTH_PERMISSION_ENABLED TRUE
    BLUETOOTH_PERMISSION_TEXT "For Bluetooth MIDI"
    DOCUMENT_BROWSER_ENABLED TRUE
)

juce_generate_juce_header(Melissa)

target_sources(Melissa PRIVATE
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
    Source/Audio/MelissaAudioEngine.cpp
    Source/Audio/MelissaBeepGenerator.cpp
    Source/Audio/MelissaBPMDetector.cpp
    Source/Audio/MelissaMetronome.cpp
    Source/Audio/soundtouch/BPMDetect_for_Melissa.cpp
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
    Source/UI/MelissaAboutComponent.h
    Source/UI/MelissaBPMSettingComponent.h
    Source/UI/MelissaButtons.h
    Source/UI/MelissaBrowserComponent.cpp
    Source/UI/MelissaBrowserComponent.h
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
    Source/UI/MelissaStemControlComponent.cpp
    Source/UI/MelissaStemControlComponent.h
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
    Submodule/demucs.cpp/include
    Submodule/demucs.cpp/src
    ${OPENBLAS_INCLUDE_DIR}
    Source
    Source/Audio
    Source/Audio/soundtouch
    Source/UI
)

target_compile_definitions(Melissa PRIVATE
    _USE_MATH_DEFINES
    MELISSA_FULL_VERSION
    JUCE_SILENCE_XCODE_15_LINKER_WARNING
    MELISSA_USE_STEM_SEPARATION
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
    tensorflow_framework
    demucs.cpp.lib
    "${OPENBLAS_LIBRARIES}"
)

juce_add_binary_data(MelissaBinaryData
    SOURCES
    Resource/icon.png
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
    Resource/Language/en-US.txt
    Resource/Language/ja-JP.txt
    Resource/logo.png
)

target_link_libraries(Melissa PRIVATE MelissaBinaryData)

if (APPLE)
    set_target_properties(Melissa PROPERTIES
        MACOSX_BUNDLE TRUE
        XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Manual"
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "-"
        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ""
        XCODE_ATTRIBUTE_PROVISIONING_PROFILE_SPECIFIER ""
        XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO"
        XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
        XCODE_EMBED_FRAMEWORKS "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libtensorflow-cpu-darwin-universal-binary-2.8.0/lib/libtensorflow_framework.2.dylib;${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libtensorflow-cpu-darwin-universal-binary-2.8.0/lib/libtensorflow.2.dylib"
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
    )
endif()

if(WIN32)
    add_custom_command(TARGET Melissa POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${OPENBLAS_ROOT}/bin/libopenblas.dll"
            $<TARGET_FILE_DIR:Melissa>
    )
endif()

target_link_options(Melissa PRIVATE
    -L${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/spleeterpp/lib
    -L${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libtensorflow-cpu-darwin-universal-binary-2.8.0/lib
    -Wl,-ld_classic
)

if(USE_OPENBLAS)
    set(BLAS_LIBRARIES "${OPENBLAS_LIBRARIES}")
    set(BLAS_FOUND TRUE)
endif()
