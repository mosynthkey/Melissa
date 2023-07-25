//
//  MelissaExportManager.cpp
//  Melissa
//
//  Copyright(c) 2023 Masaki Ono
//

#include "MelissaDataSource.h"
#include "MelissaExportManager.h"

MelissaExportManager MelissaExportManager::instance_;

MelissaExportManager::MelissaExportManager() : Thread("MelissaExportThread")
{
}

MelissaExportManager::~MelissaExportManager()
{
}

void MelissaExportManager::regist(std::unique_ptr<MelissaExporter> exporter)
{
    std::lock_guard<std::mutex> lock(mutex_);
    exporters_.emplace_back(std::move(exporter));
    if (!isThreadRunning()) startThread();
}

void MelissaExportManager::run()
{
    MelissaDataSource::getInstance()->notifyExportStarted();
    
    std::unique_ptr<MelissaExporter> exporter;
    while (!exporters_.empty())
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            exporter = std::move(exporters_.back());
            exporters_.pop_back();
        }
        exporter->exportToFile();
    }
    
    MelissaDataSource::getInstance()->notifyExportCompleted(true, TRANS("export_completed"));
}
