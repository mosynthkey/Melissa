//
//  MelissaUpdateChecker.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

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
    static String getUpdateContents();
    static UpdateStatus getUpdateStatus();
    
    // UI
    static void showUpdateDialog();
};
