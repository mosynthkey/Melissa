//
//  MelissaUpdateChecker.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include <regex>
#include "MelissaModalDialog.h"
#include "MelissaOptionDialog.h"
#include "MelissaUpdateChecker.h"

using namespace juce;

MelissaUpdateChecker::UpdateStatus MelissaUpdateChecker::status_ = kUpdateStatus_NotChecked;

#ifdef MELISSA_FULL_VERSION
static URL latestVersionURL ("https://api.github.com/repos/mosynthkey/Melissa/releases/latest");
#else
static URL latestVersionURL ("https://api.github.com/repos/mosynthkey/Melissa_Lite/releases/latest");
#endif

String MelissaUpdateChecker::getLatestVersionNumberString()
{
    std::unique_ptr<InputStream> inStream(latestVersionURL.createInputStream (URL::InputStreamOptions(URL::ParameterHandling::inAddress).withConnectionTimeoutMs(1000)));
    
    if (inStream == nullptr) return "";

    auto content = inStream->readEntireStreamAsString();
    auto latestReleaseDetails = JSON::parse(content);

    auto* json = latestReleaseDetails.getDynamicObject();
    if (json == nullptr) return "";

    auto versionString = json->getProperty ("tag_name").toString();
    if (versionString.isEmpty()) return "";

    return versionString;
}

String MelissaUpdateChecker::getUpdateContents()
{
    std::unique_ptr<InputStream> inStream(latestVersionURL.createInputStream (URL::InputStreamOptions(URL::ParameterHandling::inAddress).withConnectionTimeoutMs(1000)));
    
    if (inStream == nullptr) return "";

    auto content = inStream->readEntireStreamAsString();
    auto latestReleaseDetails = JSON::parse(content);

    auto* json = latestReleaseDetails.getDynamicObject();
    if (json == nullptr) return "";

    return json->getProperty ("body").toString();
}

MelissaUpdateChecker::UpdateStatus MelissaUpdateChecker::getUpdateStatus()
{
    if (status_ != kUpdateStatus_NotChecked) return status_;
    
    const std::string latestVersionNumberString = MelissaUpdateChecker::getLatestVersionNumberString().toStdString();
    std::regex re("v(\\d+)\\.(\\d+)\\.(\\d+)");
    std::smatch match;
    
    if (std::regex_match(latestVersionNumberString, match, re))
    {
        const int latestVersionNumber = (std::stoi(match[1].str()) << 16) | (std::stoi(match[2].str()) << 8) | std::stoi(match[3].str());
        if (latestVersionNumber > ProjectInfo::versionNumber)
        {
            status_ = kUpdateStatus_UpdateExists;
        }
        else
        {
            status_ = kUpdateStatus_IsLatest;
        }
    }
    else
    {
        status_ = kUpdateStatus_Failed;
    }
    
    return status_;
}

void MelissaUpdateChecker::showUpdateDialog()
{
    const std::vector<String> options = { TRANS("check"), TRANS("cancel") };
    const auto updateContents = getUpdateContents();
    
    auto dialog = std::make_shared<MelissaOptionDialog>(TRANS("there_is_update") + "\n\n----\n\n" + updateContents, options, [&](size_t index) {
        if (index == 0) URL("https://mosynthkey.github.io/Melissa/#download").launchInDefaultBrowser();
    });
    MelissaModalDialog::show(dialog, TRANS("update"));
}
