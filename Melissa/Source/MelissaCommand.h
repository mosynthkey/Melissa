//
//  MelissaCommand.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include <map>
#include "MelissaDataSource.h"
#include "MelissaModel.h"

class MelissaCommand
{
public:
    static void excuteCommand(const String& commandAsString, float value);
    static String getFunctionDescription(const String& commandAsString);
    
    // Singleton
    static MelissaCommand* getInstance() { return &instance_; }
    MelissaCommand(const MelissaCommand&) = delete;
    MelissaCommand& operator=(const MelissaCommand&) = delete;
    MelissaCommand(MelissaDataSource&&) = delete;
    MelissaCommand& operator=(MelissaCommand&&) = delete;
    
private:
    // Singleton
    MelissaCommand();
    ~MelissaCommand() {}
    static MelissaCommand instance_;
    
    MelissaDataSource* dataSource_;
    MelissaModel* model_;
    
    using DescriptionFunctionPair = std::pair<String, std::function<void(float)>>;
    static inline std::map<String, DescriptionFunctionPair> commands_;
};
