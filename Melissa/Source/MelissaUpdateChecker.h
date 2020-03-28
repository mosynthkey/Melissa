#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MelissaUpdateChecker
{
public:
    enum UpdateStatus
    {
        kUpdateStatus_NotChecked,
        kUpdateStatus_IsLatest,
        kUpdateStatus_UpdateExists,
        kUpdateStatus_Failed,
    };
    
    static UpdateStatus status_;
    static String getLatestVersionNumberString();
    static UpdateStatus getUpdateStatus();
};
