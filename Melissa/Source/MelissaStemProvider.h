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
    kStemProviderStatus_Available,
    kStemProviderStatus_NotAvailable,
    kStemProviderStatus_Processing,
};

enum StemProviderResult
{
    kStemProviderResult_Success,
    kStemProviderResult_FailedToReadSourceFile,
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
    
    void failedToReadPreparedStems();
    
    void prepareForLoadStems(const File& fileToOpen, File& originalFile, std::map<std::string, File>& stemFiles);
    void deleteStems();
    
    StemProviderStatus getStemProviderStatus() const { return status_; }
    StemProviderResult getStemProviderResult() const { return result_; }
    
    // Listener
    void addListener(MelissaStemProviderListener* listener);
    void removeListener(MelissaStemProviderListener* listener);
    
    // Singleton
    static MelissaStemProvider* getInstance() { return &instance_; }
    MelissaStemProvider(const MelissaStemProvider&) = delete;
    MelissaStemProvider& operator=(const MelissaStemProvider&) = delete;
    MelissaStemProvider(MelissaStemProvider&&) = delete;
    MelissaStemProvider& operator=(MelissaStemProvider&&) = delete;
    
    static inline const std::string partNames_[] = { "accompaniment", "vocals", "piano", "bass", "drums", "other" };
    
private:
    // Singleton
    MelissaStemProvider();
    ~MelissaStemProvider();
    static MelissaStemProvider instance_;
    std::vector<MelissaStemProviderListener*> listeners_;
    
    void run() override;
    StemProviderResult createStems();
    
    StemProviderStatus status_;
    StemProviderResult result_;
    
    File songFile_;
};
