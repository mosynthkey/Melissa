#ifndef MODEL_HPP
#define MODEL_HPP

#include "dsp.hpp"
#include "tensor.hpp"
#include <Eigen/Dense>
#include <array>
#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include "onnxruntime_cxx_api.h"

namespace demucsonnx
{
extern Ort::AllocatorWithDefaultOptions allocator;
extern Ort::RunOptions run_options;

// Define a type for your callback function
using ProgressCallback = std::function<void(float, const std::string &)>;

const int FREQ_BRANCH_LEN = 336;
const int TIME_BRANCH_LEN_IN = 343980;

struct demucs_model {
    std::unique_ptr<Ort::Session> sess;        // Smart pointer to allow "empty" state
    int nb_sources = 0;
    Ort::Env env{ORT_LOGGING_LEVEL_ERROR, "demucs_onnx"}; // Persistent environment
    std::vector<std::string> input_names;      // Persistent input names
    std::vector<std::string> output_names;     // Persistent output names

    std::vector<const char*> input_names_ptrs;
    std::vector<const char*> output_names_ptrs;

    // Constructor (optionally initialize here if needed)
    demucs_model() = default;
};

bool load_model(const char *model_data,
                int n_bytes,
                struct demucs_model &model,
                Ort::SessionOptions &session_options);

bool load_model(const std::vector<char> &model_data,
                struct demucs_model &model,
                Ort::SessionOptions &session_options);

struct demucs_segment_buffers
{
    int segment_samples;
    int le;
    int pad;
    int pad_end;
    int padded_segment_samples;
    int nb_stft_frames;
    int nb_stft_bins;

    Eigen::Tensor3dXf targets_out;
    Eigen::MatrixXf padded_mix;
    Eigen::Tensor3dXcf z;

    std::vector<int64_t> x_onnx_in_shape;
    std::vector<int64_t> xt_onnx_in_shape;

    std::vector<int64_t> x_onnx_out_shape;
    std::vector<int64_t> xt_onnx_out_shape;

    std::vector<Ort::Value> input_tensors;
    std::vector<Ort::Value> output_tensors;

    // constructor for demucs_segment_buffers that takes int parameters

    // let's do pesky precomputing of the signal repadding to 1/4 hop
    // for time and frequency alignment
    demucs_segment_buffers(int nb_channels, int segment_samples, int nb_sources)
        : segment_samples(segment_samples),
          le(int(std::ceil((float)segment_samples / (float)FFT_HOP_SIZE))),
          pad(std::floor((float)FFT_HOP_SIZE / 2.0f) * 3),
          pad_end(pad + le * FFT_HOP_SIZE - segment_samples),
          padded_segment_samples(segment_samples + pad + pad_end),
          nb_stft_frames(segment_samples / demucsonnx::FFT_HOP_SIZE + 1),
          nb_stft_bins(demucsonnx::FFT_WINDOW_SIZE / 2 + 1),
          targets_out(nb_sources, nb_channels, segment_samples),
          padded_mix(nb_channels, padded_segment_samples),
          z(nb_channels, nb_stft_bins, nb_stft_frames+4),
          // complex-as-channels implies 2*nb_channels for real+imag
          x_onnx_in_shape({1, 2 * nb_channels, nb_stft_bins - 1, nb_stft_frames}),
          xt_onnx_in_shape({1, nb_channels, segment_samples}),
          x_onnx_out_shape({1, nb_sources, 2 * nb_channels, nb_stft_bins - 1, nb_stft_frames}),
          xt_onnx_out_shape({1, nb_sources, nb_channels, segment_samples})
    {
        // precompute the input tensors
        // inputs in form (xt, x)
        input_tensors.push_back(Ort::Value::CreateTensor<float>(
            demucsonnx::allocator,
            xt_onnx_in_shape.data(),
            xt_onnx_in_shape.size()));

        input_tensors.push_back(Ort::Value::CreateTensor<float>(
            demucsonnx::allocator,
            x_onnx_in_shape.data(),
            x_onnx_in_shape.size()));

        // precompute the output tensors
        // outputs in form (x_out, xt_out)
        output_tensors.push_back(Ort::Value::CreateTensor<float>(
            demucsonnx::allocator,
            x_onnx_out_shape.data(),
            x_onnx_out_shape.size()));

        output_tensors.push_back(Ort::Value::CreateTensor<float>(
            demucsonnx::allocator,
            xt_onnx_out_shape.data(),
            xt_onnx_out_shape.size()));
    };
};

const float SEGMENT_LEN_SECS = 7.8;      // 8 seconds, the demucs chunk size
const float SEGMENT_OVERLAP_SECS = 0.25; // 0.25 overlap
const float MAX_SHIFT_SECS = 0.5;        // max shift
const float OVERLAP = 0.25;              // overlap between segments
const float TRANSITION_POWER = 1.0;      // transition between segments

Eigen::Tensor3dXf demucs_inference(struct demucs_model &model,
                                   const Eigen::MatrixXf &audio,
                                   ProgressCallback cb);

void model_inference(struct demucs_model &model,
                     struct demucsonnx::demucs_segment_buffers &buffers,
                     struct demucsonnx::stft_buffers &stft_buf);
} // namespace demucsonnx

#endif // MODEL_HPP
