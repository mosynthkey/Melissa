//
//  MelissaSpleeterBridge.cpp
//  Melissa
//
//  Copyright(c) 2022 Masaki Ono
//

#include "MelissaSpleeterBridge.h"
#include "MelissaDataSource.h"
#include "spleeter/spleeter.h"
#include "input_file.h"
#include "output_folder.h"
#include "utils.h"
#include "split.h"

MelissaSpleeterBridge::MelissaSpleeterBridge() : Thread("MelissaSpleeterProcessThread")
{
    
}

void MelissaSpleeterBridge::requestStems()
{
    startThread();
}

void MelissaSpleeterBridge::run()
{
    createStems();
}

void MelissaSpleeterBridge::createStems()
{
    auto dataSource = MelissaDataSource::getInstance();
    
    auto working_directory = File::getCurrentWorkingDirectory().getFullPathName().toStdString();
    
    // validate the parameters (output count)
    std::error_code err;
    spleeter::SeparationType separation_type = spleeter::FiveStems;
    
    // validate output directory
    File output_file(File(dataSource->getCurrentSongFilePath()).getParentDirectory());
    if (!(output_file.exists() && output_file.isDirectory())) {
      std::cerr << "Output folder " << output_file.getFullPathName().toRawUTF8() << " does not seem to exist" << std::endl;
      return;
    }

    // Initialize spleeter
    auto settingsDir = (File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Melissa"));
    auto model_path = settingsDir.getChildFile("models").getFullPathName().toStdString();
    spleeter::Initialize(model_path, {separation_type}, err);
    if (err) {
      std::cerr << "Couldn't initialize spleeter" << std::endl;
      return;
    }

    InputFile input(dataSource->getCurrentSongFilePath().toStdString());
    input.Open(err);
    if (err) {
      std::cerr << "Couldn't read source file" << std::endl;
      return;
    }
    
    OutputFolder output_folder(File(dataSource->getCurrentSongFilePath()).getParentDirectory().getFullPathName().toStdString());
    
    while (true) {
      auto data = input.Read();
      if (data.cols() == 0) {
        return;
      }
      auto result = Split(data, separation_type, err);
      if (err) {
        std::cerr << "Failed to split" << std::endl;
        return;
      }
      output_folder.Write(result, err);
      if (err) {
        std::cerr << "Failed to export" << std::endl;
        return;
      }
    }
    return;
}
