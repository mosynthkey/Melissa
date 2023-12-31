//
//  MelissaExportManager.h
//  Melissa
//
//  Copyright(c) 2023 Masaki Ono
//

#pragma once

#include <JuceHeader.h>
#include "MelissaExporter.h"

class MelissaExportManager : public juce::Thread
{
public:
    void regist(std::unique_ptr<MelissaExporter> exporter);
    bool isExporting() { return (exporters_.size() != 0) && isThreadRunning(); }
    
    // Singleton
    static MelissaExportManager* getInstance() { return &instance_; }
    MelissaExportManager(const MelissaExportManager&) = delete;
    MelissaExportManager& operator=(const MelissaExportManager&) = delete;
    MelissaExportManager(MelissaExportManager&&) = delete;
    MelissaExportManager& operator=(MelissaExportManager&&) = delete;
    
private:
    // Singleton
    MelissaExportManager();
    ~MelissaExportManager();
    static MelissaExportManager instance_;
    
    void run() override;
    
    std::mutex mutex_;
    std::vector<std::unique_ptr<MelissaExporter>> exporters_;
};
