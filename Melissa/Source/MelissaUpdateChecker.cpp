#include "MelissaUpdateChecker.h"

String MelissaUpdateChecker::getLatestVersionNumberString()
{
    URL latestVersionURL ("https://api.github.com/repos/mosynthkey/Melissa/releases/latest");
    std::unique_ptr<InputStream> inStream(latestVersionURL.createInputStream (false));
    
    if (inStream == nullptr) return "";

    auto content = inStream->readEntireStreamAsString();
    auto latestReleaseDetails = JSON::parse(content);

    auto* json = latestReleaseDetails.getDynamicObject();
    if (json == nullptr) return "";

    auto versionString = json->getProperty ("tag_name").toString();
    if (versionString.isEmpty()) return "";

    return versionString;
}
