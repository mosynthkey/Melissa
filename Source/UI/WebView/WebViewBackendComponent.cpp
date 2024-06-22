#include <JuceHeader.h>
#include "WebViewBackendComponent.h"

#ifndef WEB_VIEW_FROM_DEV_SERVER
#define WEB_VIEW_FROM_DEV_SERVER 1
#endif

//==============================================================================
namespace
{
    static const char* getMimeForExtension(const juce::String& extension)
    {
        static const std::unordered_map<juce::String, const char*> mimeMap =
        {
            { { "htm"   },  "text/html"                },
            { { "html"  },  "text/html"                },
            { { "txt"   },  "text/plain"               },
            { { "jpg"   },  "image/jpeg"               },
            { { "jpeg"  },  "image/jpeg"               },
            { { "svg"   },  "image/svg+xml"            },
            { { "ico"   },  "image/vnd.microsoft.icon" },
            { { "json"  },  "application/json"         },
            { { "png"   },  "image/png"                },
            { { "css"   },  "text/css"                 },
            { { "map"   },  "application/json"         },
            { { "js"    },  "text/javascript"          },
            { { "woff2" },  "font/woff2"               }
        };

        if (const auto it = mimeMap.find(extension.toLowerCase()); it != mimeMap.end())
            return it->second;

        jassertfalse;

        return "";
    }

    static juce::String getExtension(juce::String filename)
    {
        return filename.fromLastOccurrenceOf(".", false, false);
    }

    static auto streamToVector(juce::InputStream& stream)
    {
        std::vector<std::byte> result((size_t)stream.getTotalLength());

        stream.setPosition(0);

        [[maybe_unused]] const auto bytesRead = (size_t)stream.read(result.data(), result.size());

        jassert(bytesRead == result.size());

        return result;
    }

    std::vector<std::byte> convertFromStringToByteVector(const std::string& s)
    {
        std::vector<std::byte> bytes;
        bytes.reserve(std::size(s));

        std::transform(std::begin(s), std::end(s), std::back_inserter(bytes), [](char const& c) {
            return std::byte(c);
            });

        return bytes;
    }
}

//==============================================================================
// WebViewBackendComponent
//==============================================================================
WebViewBackendComponent::WebViewBackendComponent()
    : misWebViewBundle(BinaryData::webviewdummy_zip, BinaryData::webviewdummy_zipSize, false)
    , zipWebViewBundle(std::make_unique<juce::ZipFile>(misWebViewBundle))
{
    addAndMakeVisible(webComponent);

#if WEB_VIEW_FROM_DEV_SERVER
    webComponent.goToURL(SinglePageBrowser::localDevServerAddress);
#else
    webComponent.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
#endif
}

WebViewBackendComponent::~WebViewBackendComponent()
{
}

void WebViewBackendComponent::emitCustomSoundChangedEvent(const juce::var& soundName)
{
    webComponent.emitEventIfBrowserIsVisible("onCustomSoundChanged", soundName);
}

//==============================================================================
void WebViewBackendComponent::paint(juce::Graphics&)
{
}

void WebViewBackendComponent::resized()
{
    webComponent.setBounds(getLocalBounds());
}

//==============================================================================
std::optional<juce::WebBrowserComponent::Resource> WebViewBackendComponent::getResource(const juce::String& url)
{
    const auto urlToRetrive = url == "/" ? juce::String{ "index.html" }
    : url.fromFirstOccurrenceOf("/", false, false);

    if (auto* archive = zipWebViewBundle.get())
    {
        if (auto* entry = archive->getEntry(urlToRetrive))
        {
            auto stream = juce::rawToUniquePtr(archive->createStreamForEntry(*entry));
            auto v = streamToVector(*stream);
            auto mime = getMimeForExtension(getExtension(entry->filename).toLowerCase());

            return juce::WebBrowserComponent::Resource{
                std::move(v),
                std::move(mime)
            };
        }
    }

    if (urlToRetrive == "index.html")
    {
        auto fallbackIndexHtml = SinglePageBrowser::fallbackPageHtml;

        return juce::WebBrowserComponent::Resource{
            convertFromStringToByteVector(fallbackIndexHtml.toStdString()),
            juce::String { "text/html" }
        };
    }

    return std::nullopt;
}
