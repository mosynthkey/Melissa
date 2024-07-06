#include <JuceHeader.h>
#include "WebViewBackendComponent.h"

#ifndef WEB_VIEW_FROM_DEV_SERVER
#define WEB_VIEW_FROM_DEV_SERVER 1
#endif

//==============================================================================
namespace
{
    static const char *getMimeForExtension(const juce::String &extension)
    {
        static const std::unordered_map<juce::String, const char *> mimeMap =
            {
                {{"htm"}, "text/html"},
                {{"html"}, "text/html"},
                {{"txt"}, "text/plain"},
                {{"jpg"}, "image/jpeg"},
                {{"jpeg"}, "image/jpeg"},
                {{"svg"}, "image/svg+xml"},
                {{"ico"}, "image/vnd.microsoft.icon"},
                {{"json"}, "application/json"},
                {{"png"}, "image/png"},
                {{"css"}, "text/css"},
                {{"map"}, "application/json"},
                {{"js"}, "text/javascript"},
                {{"woff2"}, "font/woff2"}};

        if (const auto it = mimeMap.find(extension.toLowerCase()); it != mimeMap.end())
            return it->second;

        jassertfalse;

        return "";
    }

    static juce::String getExtension(juce::String filename)
    {
        return filename.fromLastOccurrenceOf(".", false, false);
    }

    static auto streamToVector(juce::InputStream &stream)
    {
        std::vector<std::byte> result((size_t)stream.getTotalLength());

        stream.setPosition(0);

        [[maybe_unused]] const auto bytesRead = (size_t)stream.read(result.data(), result.size());

        jassert(bytesRead == result.size());

        return result;
    }

    std::vector<std::byte> convertFromStringToByteVector(const std::string &s)
    {
        std::vector<std::byte> bytes;
        bytes.reserve(std::size(s));

        std::transform(std::begin(s), std::end(s), std::back_inserter(bytes), [](char const &c)
                       { return std::byte(c); });

        return bytes;
    }
}

//==============================================================================
// WebViewBackendComponent
//==============================================================================
WebViewBackendComponent::WebViewBackendComponent()
    : misWebViewBundle(BinaryData::webviewdummy_zip, BinaryData::webviewdummy_zipSize, false), zipWebViewBundle(std::make_unique<juce::ZipFile>(misWebViewBundle))
{
    addAndMakeVisible(webComponent);

#if WEB_VIEW_FROM_DEV_SERVER
    webComponent.goToURL(SinglePageBrowser::localDevServerAddress);
#else
    webComponent.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
#endif

    MelissaModel::getInstance()->addListener(this);
    MelissaDataSource::getInstance()->addListener(this);
}

WebViewBackendComponent::~WebViewBackendComponent()
{
    MelissaModel::getInstance()->removeListener(this);
}

//==============================================================================
void WebViewBackendComponent::paint(juce::Graphics &)
{
}

void WebViewBackendComponent::resized()
{
    webComponent.setBounds(getLocalBounds());
}

//==============================================================================
std::optional<juce::WebBrowserComponent::Resource> WebViewBackendComponent::getResource(const juce::String &url)
{
    const auto urlToRetrive = url == "/" ? juce::String{"index.html"}
                                         : url.fromFirstOccurrenceOf("/", false, false);

    if (auto *archive = zipWebViewBundle.get())
    {
        if (auto *entry = archive->getEntry(urlToRetrive))
        {
            auto stream = juce::rawToUniquePtr(archive->createStreamForEntry(*entry));
            auto v = streamToVector(*stream);
            auto mime = getMimeForExtension(getExtension(entry->filename).toLowerCase());

            return juce::WebBrowserComponent::Resource{
                std::move(v),
                std::move(mime)};
        }
    }

    if (urlToRetrive == "index.html")
    {
        auto fallbackIndexHtml = SinglePageBrowser::fallbackPageHtml;

        return juce::WebBrowserComponent::Resource{
            convertFromStringToByteVector(fallbackIndexHtml.toStdString()),
            juce::String{"text/html"}};
    }

    return std::nullopt;
}

juce::String WebViewBackendComponent::createCurrntMarkerAsJsonString()
{
    auto dataSource = MelissaDataSource::getInstance();
    std::vector<MelissaDataSource::Song::Marker> markers;
    dataSource->getMarkers(markers);

    juce::Array<juce::var> markersJson;
    for (const auto &marker : markers)
    {
        juce::DynamicObject *markerObj = new juce::DynamicObject();
        markerObj->setProperty("position", marker.position_);
        markerObj->setProperty("colour", juce::Colour(marker.colourR_, marker.colourG_, marker.colourB_).toString());
        markerObj->setProperty("memo", marker.memo_);

        markersJson.add(markerObj);
    }

    return juce::JSON::toString(markersJson);
}

void WebViewBackendComponent::playbackStatusChanged(PlaybackStatus status)
{
    juce::Array<juce::var> message = {juce::String("playbackStatusChanged"), static_cast<int>(status)};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::playbackModeChanged(PlaybackMode mode)
{
    juce::Array<juce::var> message = {juce::String("playbackModeChanged"), static_cast<int>(mode)};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::musicVolumeChanged(float volume)
{
    juce::Array<juce::var> message = {juce::String("musicVolumeChanged"), volume};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::pitchChanged(float semitone)
{
    juce::Array<juce::var> message = {juce::String("pitchChanged"), semitone};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::speedChanged(int speed)
{
    juce::Array<juce::var> message = {juce::String("speedChanged"), speed};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio)
{
    juce::Array<juce::var> message = {juce::String("loopPosChanged"), aTimeMSec, aRatio, bTimeMSec, bRatio};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::playingPosChanged(float time, float ratio)
{
    juce::Array<juce::var> message = {juce::String("playingPosChanged"), time, ratio};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::metronomeSwitchChanged(bool on)
{
    juce::Array<juce::var> message = {juce::String("metronomeSwitchChanged"), static_cast<int>(on)};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::bpmChanged(float bpm)
{
    juce::Array<juce::var> message = {juce::String("bpmChanged"), bpm};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::beatPositionChanged(float beatPositionMSec)
{
    juce::Array<juce::var> message = {juce::String("beatPositionChanged"), beatPositionMSec};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::accentChanged(int accent)
{
    juce::Array<juce::var> message = {juce::String("accentChanged"), accent};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::metronomeVolumeChanged(float volume)
{
    juce::Array<juce::var> message = {juce::String("metronomeVolumeChanged"), volume};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::musicMetronomeBalanceChanged(float balance)
{
    juce::Array<juce::var> message = {juce::String("musicMetronomeBalanceChanged"), balance};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::outputModeChanged(OutputMode outputMode)
{
    juce::Array<juce::var> message = {juce::String("outputModeChanged"), static_cast<int>(outputMode)};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::eqSwitchChanged(bool on)
{
    juce::Array<juce::var> message = {juce::String("eqSwitchChanged"), static_cast<int>(on)};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::eqFreqChanged(size_t band, float freq)
{
    juce::Array<juce::var> message = {juce::String("eqFreqChanged"), static_cast<int>(band), freq};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::eqGainChanged(size_t band, float gain)
{
    juce::Array<juce::var> message = {juce::String("eqGainChanged"), static_cast<int>(band), gain};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::eqQChanged(size_t band, float q)
{
    juce::Array<juce::var> message = {juce::String("eqQChanged"), static_cast<int>(band), q};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::playPartChanged(PlayPart playPart)
{
    juce::Array<juce::var> message = {juce::String("playPartChanged"), static_cast<int>(playPart)};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::customPartVolumeChanged(CustomPartVolume part, float volume)
{
    juce::Array<juce::var> message = {juce::String("customPartVolumeChanged"), part, volume};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::mainVolumeChanged(float mainVolume)
{
    juce::Array<juce::var> message = {juce::String("mainVolumeChanged"), mainVolume};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::preCountSwitchChanged(bool preCountSwitch)
{
    juce::Array<juce::var> message = {juce::String("preCountSwitchChanged"), static_cast<int>(preCountSwitch)};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::songChanged(const juce::String &filePath, size_t bufferLength, int32_t sampleRate)
{
    juce::Array<juce::var> message = {juce::String("songChanged"), filePath, static_cast<int>(bufferLength), static_cast<int>(sampleRate)};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::historyUpdated()
{
    juce::Array<juce::var> message = {juce::String("historyUpdated")};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::playlistUpdated(size_t index)
{
    juce::Array<juce::var> message = {juce::String("preCountSwitchChanged"), static_cast<int>(index)};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::practiceListUpdated()
{
    juce::Array<juce::var> message = { juce::String("practiceListUpdated"),
                                       createCurrntMarkerAsJsonString() };
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::markerUpdated()
{
    auto dataSource = MelissaDataSource::getInstance();
    std::vector<MelissaDataSource::Song::Marker> markers;
    dataSource->getMarkers(markers);

    juce::Array<juce::var> markersJson;
    for (const auto &marker : markers)
    {
        juce::DynamicObject *markerObj = new juce::DynamicObject();
        markerObj->setProperty("position", marker.position_);
        markerObj->setProperty("colour", juce::Colour(marker.colourR_, marker.colourG_, marker.colourB_).toString());
        markerObj->setProperty("memo", marker.memo_);

        markersJson.add(markerObj);
    }

    juce::Array<juce::var> message = {juce::String("markerUpdated"), juce::JSON::toString(markersJson)};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::fileLoadStatusChanged(FileLoadStatus status, const juce::String &filePath)
{
    juce::Array<juce::var> message = {juce::String("fileLoadStatusChanged"), static_cast<int>(status), filePath};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::shortcutUpdated()
{
    juce::Array<juce::var> message = {juce::String("shortcutUpdated")};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::colourChanged(const juce::Colour &mainColour, const juce::Colour &subColour, const juce::Colour &accentColour, const juce::Colour &textColour, const juce::Colour &waveformColour)
{
}

void WebViewBackendComponent::fontChanged(const juce::Font &mainFont, const juce::Font &subFont, const juce::Font &miniFont)
{
}

void WebViewBackendComponent::exportStarted()
{
    juce::Array<juce::var> message = {juce::String("exportStarted")};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}

void WebViewBackendComponent::exportCompleted(bool result, juce::String exportMessage)
{
    juce::Array<juce::var> message = {juce::String("exportCompleted"), result, exportMessage};
    webComponent.emitEventIfBrowserIsVisible("MelissaNotification", message);
}
