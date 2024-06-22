#include <JuceHeader.h>
#include "SinglePageBrowser.h"

//==============================================================================
#if JUCE_ANDROID
// The localhost is available on this address to the emulator
const juce::String SinglePageBrowser::localDevServerAddress = "http://10.0.2.2:3000/";
#else
const juce::String SinglePageBrowser::localDevServerAddress = "http://localhost:8080/";
#endif

const juce::String SinglePageBrowser::fallbackPageHtml = R"(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8" />
    <title>WebViewPluginDemo</title>
    <style type="text/css">
      body {
        background-color: white;
      }
    </style>
  </head>
  <body>
    <h1>WebViewPluginDemo</h1>
    <p>
      This document is a placeholder for the GUI component of the
      WebViewPluginDemo.
    </p>
    <p>
      To build the fully fledged user interface you need to install
      <a href="https://nodejs.org">node.js</a>
    </p>
    <p>
      Then navigate into the
      <code>examples/Plugins/WebViewPluginDemoGUI</code> directory inside your JUCE
      directory, and issue the following commands.
    </p>
    <pre>
        npm install
        npm run build
        npm run zip
      </pre
    >
    <p>
      This will build the full GUI package and place it in the
      <code>Assets</code> directory.
    </p>
    <p>After this, rebuild and restart this demo.</p>
  </body>
</html>
)";

//==============================================================================
bool SinglePageBrowser::pageAboutToLoad (const juce::String& newURL)
{
    return newURL == localDevServerAddress || newURL == getResourceProviderRoot();
}

