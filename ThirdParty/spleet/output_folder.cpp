/*
  ==============================================================================

    output_folder.cpp
    Created: 8 Jun 2020 7:37:19pm
    Author:  gvne

  ==============================================================================
*/

#include "output_folder.h"
#include <algorithm>
#include "constant.h"

namespace {

std::string JoinPath(const std::string parent, const std::string &child) {
    return parent + "/" + child;
}

template <typename SignalType>
void Write(SignalType signal, OutputFolder::MultiChannelFloatAudioBuffer& buffer) {
    for (auto channel_idx = 0; channel_idx < signal.rows(); channel_idx++) {
        std::vector<float> channel_data(signal.cols());
        Eigen::Map<Eigen::VectorXf>(channel_data.data(), signal.cols()) = signal.row(channel_idx);
        buffer[channel_idx].insert(buffer[channel_idx].end(), channel_data.begin(), channel_data.end());
    }
}

} // namespace

OutputFolder::OutputFolder(const std::string &path, const std::string &fileNamePrefix, int outputSampleRate, int bufferLength) :
path_(path), fileNamePrefix_(fileNamePrefix), outputSampleRate_(outputSampleRate), bufferLength_(bufferLength) {}

OutputFolder::~OutputFolder() { Flush(); }

void OutputFolder::Flush() {
    // write the remaining data
    for (auto previous_write : previous_write_) {
        ::Write(previous_write.second, buffers_[previous_write.first]);
    }
    
    // Convert sample rate and write to file
    for (auto& partNameAndBuffer : buffers_)
    {
        auto& part = partNameAndBuffer.first;
        auto& buffer = partNameAndBuffer.second;
        
        const int numChannels = static_cast<int>(buffer.size());
        if (numChannels < 1 || 2 < numChannels) return; // Do nothing
        
        float* floatBuffer[2 /* Stereo */];
        if (buffers_.size() < 2)
        {
            // Mono
            floatBuffer[0] = floatBuffer[1] = buffer[0].data();
        }
        else
        {
            // Stereo
            floatBuffer[0] = buffer[0].data();
            floatBuffer[1] = buffer[1].data();
        }
        
        AudioBuffer<float> audioBuffer(floatBuffer, numChannels, static_cast<int>(buffer[0].size()));
        MemoryAudioSource audioSource(audioBuffer, false);
        ResamplingAudioSource resamplingAudioSource(&audioSource, false, numChannels);
        resamplingAudioSource.setResamplingRatio(kProcessSamplingRate / outputSampleRate_);
        resamplingAudioSource.prepareToPlay(2048, outputSampleRate_);
        
        OggVorbisAudioFormat format;
        
        File output_file(::JoinPath(path_, fileNamePrefix_ + "_" + part + ".ogg"));
        // If file already exists, delete it
        if (output_file.existsAsFile()) output_file.deleteFile();
        
        auto writer = format.createWriterFor(new FileOutputStream(output_file), outputSampleRate_, numChannels, 16, StringPairArray(), 0);
        writer->writeFromAudioSource(resamplingAudioSource, bufferLength_ * 2);
        writer->flush();
    }
}

void OutputFolder::Write(const std::map<std::string, spleeter::Waveform> &data,
                         std::error_code &err) {
    for (auto waveform : data) {
        auto channel_count = waveform.second.rows();
        auto frame_count = waveform.second.cols();
        
        // If no writer found, create it
        if (buffers_.find(waveform.first) == std::end(buffers_)) {
            buffers_[waveform.first].resize(channel_count);
            previous_write_[waveform.first] = spleeter::Waveform();
        }
        
        // Cross fade the beginning with the kept buffer
        auto previous_write_frame_count = previous_write_[waveform.first].cols();
        Eigen::VectorXf fade_in(previous_write_frame_count);
        Eigen::VectorXf fade_out(previous_write_frame_count);
        for (auto idx = 0; idx < previous_write_frame_count; idx++) {
            fade_in(idx) = static_cast<float>(idx) / previous_write_frame_count;
            fade_out(idx) = 1.0 - fade_in(idx);
        }
        for (auto row = 0; row < previous_write_[waveform.first].rows(); row++) {
            auto data =
            waveform.second.row(row).segment(0, previous_write_frame_count);
            auto previous_data = previous_write_[waveform.first].row(row);
            data.array() = (data.array() * fade_in.transpose().array()) +
            (previous_data.array() * fade_out.transpose().array());
        }
        
        // Keep the last second
        auto frame_to_keep =
        std::min(static_cast<int>(kProcessSamplingRate * kBatchOverlapSeconds),
                 static_cast<int>(waveform.second.cols()));
        previous_write_[waveform.first] = waveform.second.rightCols(frame_to_keep);
        
        // Write to disk
        auto frame_to_write = frame_count - frame_to_keep;
        ::Write(waveform.second.leftCols(frame_to_write), buffers_[waveform.first]);
    }
}
