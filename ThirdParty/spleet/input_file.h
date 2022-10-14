/*
  ==============================================================================

    input_file.h
    Created: 8 Jun 2020 7:32:03pm
    Author:  gvne

  ==============================================================================
*/

#pragma once

#include <string>
#include <JuceHeader.h>
#include "spleeter/spleeter.h"

class InputFile {
 public:
  InputFile(const std::string& path);
  
  /// Open the file for reading
  void Open(std::error_code& err);
  
  /// Read at most 1minute of audio and convert it to stereo 44100Hz
  /// Each read returns the last 1 second of the previous read
  spleeter::Waveform Read();
  
 private:
  std::string path_;
  std::shared_ptr<AudioFormatReaderSource> reader_;
  double source_sampling_rate_;
  uint64_t source_frame_count_;
  uint8_t source_channel_count_;
  uint64_t last_end_of_frame_;
  bool end_of_file_;
};
