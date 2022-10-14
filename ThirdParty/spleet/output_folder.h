/*
  ==============================================================================

    output_folder.h
    Created: 8 Jun 2020 7:37:19pm
    Author:  gvne

  ==============================================================================
*/

#pragma once

#include <map>
#include <string>
#include <vector>
#include <JuceHeader.h>
#include "spleeter/spleeter.h"

class OutputFolder {
public:
  OutputFolder(const std::string &path, const std::string &fileNamePrefix, int outputSampleRate, int bufferLength);
  ~OutputFolder();

  /// Flush the remaining data
  void Flush();

  /// Write data and cross fade the last 1second with the end of the previous
  /// input
  void Write(const std::map<std::string, spleeter::Waveform> &data,
             std::error_code &err);
    
  // Add for Melissa
  using MultiChannelFloatAudioBuffer = std::vector<std::vector<float>>;
  void SetFileNamePrefix(const std::string& fileNamePrefix) { fileNamePrefix_ = fileNamePrefix; }
  
 private:
  std::string path_;
  std::string fileNamePrefix_;
  int outputSampleRate_;
  int bufferLength_;
  
  std::map<std::string, MultiChannelFloatAudioBuffer> buffers_;
  std::map<std::string, spleeter::Waveform> previous_write_;
};
