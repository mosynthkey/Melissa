/*
  ==============================================================================

    io.cpp
    Created: 1 Jun 2020 2:33:22pm
    Author:  gvne

  ==============================================================================
*/

#include "io.h"
#include <JuceHeader.h>

void Read(const std::string &path, Eigen::MatrixXf *data, double *sampling_rate,
          std::error_code &err) {
  String file(path);
  AudioFormatManager formatManager;
  formatManager.registerBasicFormats();
  auto reader = std::make_unique<AudioFormatReaderSource>(
      formatManager.createReaderFor(file), true);
  auto sample_reader = reader->getAudioFormatReader();

  *sampling_rate = sample_reader->sampleRate;
  data->resize(sample_reader->numChannels, sample_reader->lengthInSamples);

  // Alloc data
  std::vector<float *> array_data;
  std::vector<std::vector<float>> vec_data;
  for (auto channel_idx = 0; channel_idx < sample_reader->numChannels;
       channel_idx++) {
    std::vector<float> channel_data(sample_reader->lengthInSamples);
    array_data.push_back(channel_data.data());
    vec_data.emplace_back(std::move(channel_data));
  }

  // Read
  if (!sample_reader->read(array_data.data(), sample_reader->numChannels, 0,
                           static_cast<int>(sample_reader->lengthInSamples))) {
    err = std::make_error_code(std::errc::io_error);
    return;
  }

  // Fill in waveform
  for (auto channel_idx = 0; channel_idx < data->rows(); channel_idx++) {
    data->row(channel_idx) = Eigen::Map<Eigen::VectorXf>(
        vec_data[channel_idx].data(), vec_data[channel_idx].size());
  }
}

void Write(const Eigen::MatrixXf &data, double sampling_rate,
           const std::string &destination, std::error_code &err) {
  File output_file(destination);
  WavAudioFormat format;

  int channel_count = data.rows();
  int frame_count = data.cols();

  auto writer = std::unique_ptr<AudioFormatWriter>(format.createWriterFor(new FileOutputStream(output_file), sampling_rate,
                                                                            channel_count, 16, StringPairArray(), 0));

  if (!writer) {
    err = std::make_error_code(std::errc::io_error);
    return;
  }

  AudioSampleBuffer buffer(channel_count, frame_count);
  for (auto channel_idx = 0; channel_idx < channel_count; channel_idx++) {
    Eigen::VectorXf row = data.row(channel_idx);
    buffer.copyFrom(channel_idx, 0, row.data(), frame_count);
  }
  writer->writeFromAudioSampleBuffer(buffer, 0, frame_count);
}
