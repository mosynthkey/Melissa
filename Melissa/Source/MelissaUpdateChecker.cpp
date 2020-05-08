//
//  MelissaUpdateChecker.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaModalDialog.h"
#include "MelissaOptionDialog.h"
#include "MelissaUpdateChecker.h"

MelissaUpdateChecker::UpdateStatus MelissaUpdateChecker::status_ = kUpdateStatus_NotChecked;

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

MelissaUpdateChecker::UpdateStatus MelissaUpdateChecker::getUpdateStatus()
{
    if (status_ != kUpdateStatus_NotChecked) return status_;
    
    const String latestVersionNumberString = MelissaUpdateChecker::getLatestVersionNumberString();
    
    if (latestVersionNumberString.isEmpty())
    {
        status_ = kUpdateStatus_Failed;
    }
    else if (latestVersionNumberString == (String("v") + ProjectInfo::versionString))
    {
        status_ = kUpdateStatus_IsLatest;
    }
    else
    {
        status_ = kUpdateStatus_UpdateExists;
    }
    
    return status_;
}

void MelissaUpdateChecker::showUpdateDialog()
{
    const std::vector<String> options = { TRANS("check"), TRANS("cancel") };
    auto dialog = std::make_shared<MelissaOptionDialog>(TRANS("there_is_update"), options, [&](size_t index) {
        if (index == 0) URL("https://mosynthkey.github.io/Melissa/#download").launchInDefaultBrowser();
    });
    MelissaModalDialog::show(dialog, TRANS("update"));
}
