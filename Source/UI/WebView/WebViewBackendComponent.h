#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "SinglePageBrowser.h"
#include "MelissaCommand.h"
#include "MelissaModel.h"
#include "MelissaMobileSupport.h"

//==============================================================================
class WebViewBackendComponent : public juce::Component, public MelissaModelListener, public MelissaDataSourceListener
{
public:
    //==============================================================================
    explicit WebViewBackendComponent();
    ~WebViewBackendComponent() override;

    void emitCustomSoundChangedEvent(const juce::var &soundName);

private:
    //==============================================================================
    void paint(juce::Graphics &) override;
    void resized() override;

    //==============================================================================
    std::optional<juce::WebBrowserComponent::Resource> getResource(const juce::String &url);

    //==============================================================================
    juce::MemoryInputStream misWebViewBundle;
    std::unique_ptr<juce::ZipFile> zipWebViewBundle;

    SinglePageBrowser webComponent{
        juce::WebBrowserComponent::Options{}
            .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
            .withWinWebView2Options(juce::WebBrowserComponent::Options::WinWebView2{}
                                        .withUserDataFolder(juce::File::getSpecialLocation(juce::File::SpecialLocationType::tempDirectory)))
            .withNativeIntegrationEnabled()
            .withNativeFunction("excuteCommand",
                                [safe_this = juce::Component::SafePointer(this), this](const juce::Array<juce::var> &args, std::function<void(juce::var)> complete)
                                    -> void
                                {
                                    if (safe_this.getComponent() == nullptr)
                                    {
                                        complete(juce::var(""));
                                        return;
                                    }

                                    const auto commandAsStr = args[0].toString();
                                    const auto value = static_cast<float>(args[1]);
                                    MelissaCommand::getInstance()->excuteCommand(commandAsStr, value);
                                    complete(juce::var(""));
                                    return;
                                })
            .withNativeFunction("requestWaveform",
                                [safe_this = juce::Component::SafePointer(this), this](const juce::Array<juce::var> &args, std::function<void(juce::var)> complete)
                                    -> void
                                {
                                    if (safe_this.getComponent() == nullptr)
                                    {
                                        complete(juce::var(""));
                                        return;
                                    }

                                    constexpr int kNumStrips = 1000;
                                    float strips[kNumStrips];

                                    auto dataSource = MelissaDataSource::getInstance();
                                    const size_t bufferLength = dataSource->getBufferLength();

                                    if (bufferLength == 0)
                                    {
                                        complete(juce::var(""));
                                        return;
                                    }

                                    float preview, previewMax = 0.f;

                                    const size_t stripSize = bufferLength / kNumStrips;

                                    auto lCh = std::make_unique<float[]>(stripSize + 1);
                                    auto rCh = std::make_unique<float[]>(stripSize + 1);
                                    float *audioData[] = {lCh.get(), rCh.get()};

                                    for (int32_t stripIndex = 0; stripIndex < kNumStrips; ++stripIndex)
                                    {
                                        preview = 0.f;

                                        const size_t startIndex = stripIndex * stripSize;
                                        const size_t endIndex = std::min<size_t>(startIndex + stripSize - 1, bufferLength - 1);
                                        const size_t numSamplesToRead = endIndex - startIndex + 1;

                                        dataSource->readBuffer(MelissaDataSource::kReader_Waveform, startIndex, static_cast<int>(numSamplesToRead), kPlayPart_All, audioData);
                                        for (int sampleIndex = 0; sampleIndex < numSamplesToRead; ++sampleIndex)
                                        {
                                            preview += (lCh[sampleIndex] * lCh[sampleIndex] + rCh[sampleIndex] * rCh[sampleIndex]);
                                        }
                                        preview /= numSamplesToRead;
                                        if (preview >= 1.f)
                                            preview = 1.f;
                                        if (previewMax < preview)
                                            previewMax = preview;
                                        strips[stripIndex] = preview;
                                    }

                                    // normalize
                                    for (int stripIndex = 0; stripIndex < kNumStrips; ++stripIndex)
                                    {
                                        strips[stripIndex] = strips[stripIndex] / previewMax;
                                    }

                                    juce::var wavefromAsVar;
                                    for (auto &&data : strips)
                                        wavefromAsVar.append(data);

                                    complete(juce::JSON::toString(wavefromAsVar));
                                    return;
                                })
            .withNativeFunction("getCurrentValue",
                                [safe_this = juce::Component::SafePointer(this), this](const juce::Array<juce::var> &args, std::function<void(juce::var)> complete)
                                    -> void
                                {
                                    if (safe_this.getComponent() == nullptr)
                                    {
                                        complete(juce::var(""));
                                        return;
                                    }

                                    const auto requestAsString = args[0].toString();
                                    float result = 0.f;

                                    auto model = MelissaModel::getInstance();
                                    // auto dataSource = MelissaDataSource::getInstance();
                                    if (requestAsString == "getPlayingPosRatio")
                                    {
                                        result = model->getPlayingPosRatio();
                                    }
                                    else if (requestAsString == "getLengthMSec")
                                    {
                                        result = model->getLengthMSec();
                                    }

                                    complete(juce::var(result));
                                    return;
                                })
            .withNativeFunction("getFileList",
                                [safe_this = juce::Component::SafePointer(this), this](const juce::Array<juce::var> &args, std::function<void(juce::var)> complete)
                                    -> void
                                {
                                    if (safe_this.getComponent() == nullptr)
                                    {
                                        complete(juce::var(""));
                                        return;
                                    }
                                    auto fileList = MelissaMobileSupport::getFileList();
                                    juce::var fileListAsVar;
                                    for (auto &&file : fileList)
                                        fileListAsVar.append(file.getFullPathName());
                                    
                auto listAsJSON = juce::JSON::toString(fileListAsVar);
                                
                        
                                    complete(juce::JSON::toString(fileListAsVar));
                                })
            .withNativeFunction("loadFile",
                                [safe_this = juce::Component::SafePointer(this), this](const juce::Array<juce::var> &args, std::function<void(juce::var)> complete)
                                    -> void
                                {
                                    if (safe_this.getComponent() == nullptr || args.size() == 0)
                                    {
                                        complete(juce::var(""));
                                        return;
                                    }

                                    juce::String filePath = args[0].toString();
                                    
                                    juce::String userDirAsString = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getFullPathName();
                
                                    if (filePath.startsWith(userDirAsString))
                                    {
                                        MelissaDataSource::getInstance()->loadFileAsync(filePath);
                                    }
                                    else
                                    {
                                        auto filePathInSandBox = MelissaMobileSupport::importFile(filePath);
                                        MelissaDataSource::getInstance()->loadFileAsync(filePathInSandBox);
                                    }
                
                                    complete(juce::var(""));
                                })
            .withResourceProvider([this](const auto &url)
                                  { return getResource(url); },
                                  juce::URL{SinglePageBrowser::localDevServerAddress}.getOrigin())};

    // MelissaModelListener
    void playbackStatusChanged(PlaybackStatus status) override;
    void playbackModeChanged(PlaybackMode mode) override;
    void musicVolumeChanged(float volume) override;
    void pitchChanged(float semitone) override;
    void speedChanged(int speed) override;
    void loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio) override;
    void playingPosChanged(float time, float ratio) override;
    void metronomeSwitchChanged(bool on) override;
    void bpmChanged(float bpm) override;
    void beatPositionChanged(float beatPositionMSec) override;
    void accentChanged(int accent) override;
    void metronomeVolumeChanged(float volume) override;
    void musicMetronomeBalanceChanged(float balance) override;
    void outputModeChanged(OutputMode outputMode) override;
    void eqSwitchChanged(bool on) override;
    void eqFreqChanged(size_t band, float freq) override;
    void eqGainChanged(size_t band, float gain) override;
    void eqQChanged(size_t band, float q) override;
    void playPartChanged(PlayPart playPart) override;
    void customPartVolumeChanged(CustomPartVolume part, float volume) override;
    void mainVolumeChanged(float mainVolume) override;
    void preCountSwitchChanged(bool preCountSwitch) override;

    // MelissaDataSourceListener
    void songChanged(const juce::String &filePath, size_t bufferLength, int32_t sampleRate) override;
    void historyUpdated() override;
    void playlistUpdated(size_t index) override;
    void practiceListUpdated() override;
    void markerUpdated() override;
    void fileLoadStatusChanged(FileLoadStatus status, const juce::String &filePath) override;
    void shortcutUpdated() override;
    void colourChanged(const juce::Colour &mainColour, const juce::Colour &subColour, const juce::Colour &accentColour, const juce::Colour &textColour, const juce::Colour &waveformColour) override;
    void fontChanged(const juce::Font &mainFont, const juce::Font &subFont, const juce::Font &miniFont) override;
    void exportStarted() override;
    void exportCompleted(bool result, juce::String message) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WebViewBackendComponent)
};
