/*
  ==============================================================================

    input_file.cpp
    Created: 8 Jun 2020 7:32:03pm
    Author:  gvne

  ==============================================================================
*/

#include "input_file.h"
#include "utils.h"
#include "constant.h"

InputFile::InputFile(const std::string &path)
: path_(path), last_end_of_frame_(0), end_of_file_(false) {}

void InputFile::Open(std::error_code &err) {
  String file(path_);
  AudioFormatManager formatManager;
  formatManager.registerBasicFormats();
  reader_ = std::make_shared<AudioFormatReaderSource>(
      formatManager.createReaderFor(file), true);

  if (!reader_) {
    err = std::make_error_code(std::errc::io_error);
    return;
  }

  auto sample_reader = reader_->getAudioFormatReader();
  source_sampling_rate_ = sample_reader->sampleRate;
  source_frame_count_ = sample_reader->lengthInSamples;
  source_channel_count_ = sample_reader->numChannels;
}

spleeter::Waveform InputFile::Read() {
  if (last_end_of_frame_ == source_frame_count_) {
    return spleeter::Waveform();
  }

  auto sample_reader = reader_->getAudioFormatReader();
  auto frame_index = static_cast<uint64_t>(
      std::max(last_end_of_frame_ - source_sampling_rate_ * kBatchOverlapSeconds, 0.0));
  auto frame_count = static_cast<uint64_t>(source_sampling_rate_ * kBatchSizeSeconds);
  if (frame_index + frame_count > source_frame_count_) {
    frame_count = static_cast<uint64_t>(source_frame_count_ - frame_index);
  }

  std::vector<float *> array_data;
  std::vector<std::vector<float>> vec_data;
  for (auto channel_idx = 0; channel_idx < source_channel_count_;
       channel_idx++) {
    std::vector<float> channel_data(frame_count);
    array_data.push_back(channel_data.data());
    vec_data.emplace_back(std::move(channel_data));
  }

  sample_reader->read(array_data.data(), static_cast<int>(array_data.size()), static_cast<int>(frame_index), static_cast<int>(frame_count));

  Eigen::MatrixXf data(source_channel_count_, frame_count);
  for (auto channel_idx = 0; channel_idx < data.rows(); channel_idx++) {
    data.row(channel_idx) = Eigen::Map<Eigen::VectorXf>(
        vec_data[channel_idx].data(), vec_data[channel_idx].size());
  }

  last_end_of_frame_ = frame_index + frame_count;
  return Resample(Stereo(data), source_sampling_rate_, kProcessSamplingRate);
}
