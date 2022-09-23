//
//  MelissaStemProvider.h
//  Melissa
//
//  Copyright(c) 2022 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

enum StemProviderStatus
{
    kStemProviderStatus_Ready,
    kStemProviderStatus_Processing,
};

enum StemProviderResult
{
    kStemProviderResult_Success,
    kStemProviderResult_CouldntReadSourceFile,
    kStemProviderResult_FailedToInitialize,
    kStemProviderResult_FailedToSplit,
    kStemProviderResult_FailedToExport,
    kStemProviderResult_Interrupted,
    kStemProviderResult_UnknownError
};

class MelissaStemProviderListener
{
public:
    virtual ~MelissaStemProviderListener() {}
    
    virtual void stemProviderStatusChanged(StemProviderStatus status) {}
    virtual void stemProviderResultReported(StemProviderResult result) {}
};

class MelissaStemProvider : public Thread
{
public:
    bool requestStems(const File& file);
    void getStemFiles(const File& fileToOpen, File& originalFile, std::map<std::string, File>& stemFiles);
    
    // Listener
    void addListener(MelissaStemProviderListener* listener);
    void removeListener(MelissaStemProviderListener* listener);
    
    StemProviderStatus getStatus() { return status_; }
    
    // Singleton
    static MelissaStemProvider* getInstance() { return &instance_; }
    MelissaStemProvider(const MelissaStemProvider&) = delete;
    MelissaStemProvider& operator=(const MelissaStemProvider&) = delete;
    MelissaStemProvider(MelissaStemProvider&&) = delete;
    MelissaStemProvider& operator=(MelissaStemProvider&&) = delete;
    
    static inline const std::string partNames_[] = { "vocals", "piano", "bass", "drums", "other" };
    
private:
    // Singleton
    MelissaStemProvider();
    ~MelissaStemProvider();
    static MelissaStemProvider instance_;
    std::vector<MelissaStemProviderListener*> listeners_;
    
    void run() override;
    StemProviderResult createStems();
    
    StemProviderStatus status_;
    File songFile_;
    StemProviderResult result_;
};
