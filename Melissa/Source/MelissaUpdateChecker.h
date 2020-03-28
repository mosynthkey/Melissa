#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MelissaUpdateChecker
{
public:
    enum UpdateStatus
    {
        kUpdateStatus_IsLatest,
        kUpdateStatus_UpdateExists,
        kUpdateStatus_Failed,
    };
    static String getLatestVersionNumberString();
    static UpdateStatus getUpdateStatus();
};
