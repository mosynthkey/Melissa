#include "demucs.hpp"
#include "dsp.hpp"
#include <Eigen/Dense>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <tuple>
#include <unsupported/Eigen/FFT>
#include <unsupported/Eigen/MatrixFunctions>
#include <vector>
#include <Eigen/Dense>
#include "onnxruntime_cxx_api.h"
#include <unsupported/Eigen/CXX11/Tensor>
#include "demucs.hpp"

namespace demucsonnx {
    Ort::AllocatorWithDefaultOptions allocator;
    Ort::RunOptions run_options;
}

// Core function to create and load model from in-memory data (byte array)
bool demucsonnx::load_model(
    const char* model_data,
    int n_bytes,
    struct demucsonnx::demucs_model &model,
    Ort::SessionOptions &session_options)
{
    const uint8_t* final_data = reinterpret_cast<const uint8_t*>(model_data);
    size_t final_size = n_bytes;

    model.sess = std::make_unique<Ort::Session>(model.env, final_data, final_size, session_options);

    std::vector<Ort::AllocatedStringPtr> input_name_allocs;
    input_name_allocs.push_back(model.sess->GetInputNameAllocated(0, demucsonnx::allocator));
    input_name_allocs.push_back(model.sess->GetInputNameAllocated(1, demucsonnx::allocator));

    model.input_names.push_back(input_name_allocs[0].get());  // Store as std::string
    model.input_names.push_back(input_name_allocs[1].get());

    std::vector<Ort::AllocatedStringPtr> output_name_allocs;
    output_name_allocs.push_back(model.sess->GetOutputNameAllocated(0, demucsonnx::allocator));
    output_name_allocs.push_back(model.sess->GetOutputNameAllocated(1, demucsonnx::allocator));

    model.output_names.push_back(output_name_allocs[0].get());
    model.output_names.push_back(output_name_allocs[1].get());

    for (const auto& name : model.input_names) {
        model.input_names_ptrs.push_back(name.c_str());
    }
    for (const auto& name : model.output_names) {
        model.output_names_ptrs.push_back(name.c_str());
    }

    auto output0_shape = model.sess->GetOutputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
    auto output1_shape = model.sess->GetOutputTypeInfo(1).GetTensorTypeAndShapeInfo().GetShape();

    // second dimension of output shape should match each other
    if (output0_shape[1] != output1_shape[1]) {
        return false;
    }

    model.nb_sources = output0_shape[1];
    return true;
}

// Overload for std::vector<char>
bool demucsonnx::load_model(
    const std::vector<char> &model_data,
    struct demucsonnx::demucs_model &model,
    Ort::SessionOptions &session_options)
{
    return load_model(
        model_data.data(), model_data.size(), model, session_options);
}

void RunONNXInference(
    struct demucsonnx::demucs_model &model,
    struct demucsonnx::demucs_segment_buffers &buffers
) {
    // Run the model
    model.sess->Run(
        demucsonnx::run_options,
        model.input_names_ptrs.data(),
        buffers.input_tensors.data(),
        buffers.input_tensors.size(),
        model.output_names_ptrs.data(),
        buffers.output_tensors.data(),
        model.output_names_ptrs.size()
    );
}

// run core demucs inference using onnx
void demucsonnx::model_inference(
    struct demucsonnx::demucs_model &model,
    struct demucsonnx::demucs_segment_buffers &buffers,
    struct demucsonnx::stft_buffers &stft_buf)
{
    // let's get a stereo complex spectrogram first
    demucsonnx::stft(stft_buf, buffers.padded_mix, buffers.z);

    // x = mag = z.abs(), but for CaC we're simply stacking the complex
    // spectrogram along the channel dimension

    // prepare frequency branch input by copying buffers.z into input_tensors[1]
    float *x_onnx_data = buffers.input_tensors[1].GetTensorMutableData<float>();

    int z_loop_dim_0 = buffers.z.dimension(0);

    // limiting to j-1 because we're dropping 2049 to 2048 bins
    int z_loop_dim_1 = buffers.z.dimension(1) - 1;

    // we're also dropping 2 bins from start and end i.e. the 2:2+le removal in python
    int z_loop_dim_2 = buffers.z.dimension(2) - 4;

    for (int i = 0; i < z_loop_dim_0; ++i)
    {
        for (int j = 0; j < z_loop_dim_1; ++j)
        {
            for (int k = 0; k < z_loop_dim_2; ++k)
            {
                int real_index = 2 * i * z_loop_dim_1 * z_loop_dim_2 + j * z_loop_dim_2 + k;
                int imag_index = (2 * i + 1) * z_loop_dim_1 * z_loop_dim_2 + j * z_loop_dim_2 + k;
                x_onnx_data[real_index] = buffers.z(i, j, k + 2).real();
                x_onnx_data[imag_index] = buffers.z(i, j, k + 2).imag();
            }
        }
    }

    // prepare time branch input by copying buffers.mix into  input_tensors[0]
    float *xt_onnx_data = buffers.input_tensors[0].GetTensorMutableData<float>();

    for (int i = 0; i < buffers.padded_mix.rows(); ++i)
    {
        for (int j = 0; j < buffers.segment_samples; ++j)
        {
            // calculate destination index, simple row major calculation
            // given the onnx shape of (1, 2, segment_samples)
            int dest_index = i * buffers.segment_samples + j;
            xt_onnx_data[dest_index] = buffers.padded_mix(i, j + buffers.pad);
        }
    }

    // now we have the stft, apply the core demucs inference
    // (where we removed the stft/istft to successfully convert to ONNX)
    RunONNXInference(model, buffers);

    std::cout << "ONNX inference completed." << std::endl;

    int nb_out_sources = model.nb_sources;

    // nb_sources sources, 2 channels, N samples
    std::vector<Eigen::MatrixXf> xt_3d(
        nb_out_sources,
        Eigen::MatrixXf(2, buffers.segment_samples)
    );

    // distribute the channels of buffers.x into x_4d
    // in pytorch it's (16, 2048, 336) i.e. (chan, freq, time)
    // then apply `.view(4, -1, freq, time)

    // Map output onnx tensors
    float* xt_out_data = buffers.output_tensors[1].GetTensorMutableData<float>();
    float* x_out_data = buffers.output_tensors[0].GetTensorMutableData<float>();

    for (int s = 0; s < nb_out_sources; ++s)
    { // loop over 4 sources
        for (int i = 0; i < 2; ++i)
        {
            for (int j = 0; j < buffers.segment_samples; ++j)
            {
                int index = s * 2 * buffers.segment_samples + i * buffers.segment_samples + j;
                xt_3d[s](i, j) = xt_out_data[index];
            }
        }
    }

    // If `cac` is True, `m` is actually a full spectrogram and `z` is ignored.
    // undo complex-as-channels by splitting the 2nd dim of x_4d into (2, 2)
    for (int source = 0; source < nb_out_sources; ++source)
    {
        Eigen::Tensor3dXcf z_target = Eigen::Tensor3dXcf(
            2, buffers.z.dimension(1), buffers.z.dimension(2));

        // in the CaC case, we're simply unstacking the complex
        // spectrogram from the channel dimension
        for (int i = 0; i < z_loop_dim_0; ++i)
        {
            for (int j = 0; j < z_loop_dim_1; ++j)
            {
                for (int k = 0; k < z_loop_dim_2; ++k)
                {
                    int real_index = source * 2 * z_loop_dim_0 * z_loop_dim_1 * z_loop_dim_2 + 2 * i * z_loop_dim_1 * z_loop_dim_2 + j * z_loop_dim_2 + k;
                    int imag_index = source * 2 * z_loop_dim_0 * z_loop_dim_1 * z_loop_dim_2 +  (2 * i + 1) * z_loop_dim_1 * z_loop_dim_2 + j * z_loop_dim_2 + k;
                    z_target(i, j, k + 2) =
                        std::complex<float>(x_out_data[real_index],
                                            x_out_data[imag_index]);
                }
                // set the first two and last two bins to zero
                z_target(i, j, 0) = std::complex<float>(0.0f, 0.0f);
                z_target(i, j, 1) = std::complex<float>(0.0f, 0.0f);
                z_target(i, j, buffers.z.dimension(2) - 1) = std::complex<float>(0.0f, 0.0f);
                z_target(i, j, buffers.z.dimension(2) - 2) = std::complex<float>(0.0f, 0.0f);
            }

            // set the entire slice along the last bin to zero
            for (int k = 0; k < buffers.z.dimension(2); ++k)
            {
                z_target(i, buffers.z.dimension(1) - 1, k) = std::complex<float>(0.0f, 0.0f);
            }
        }

        Eigen::MatrixXf padded_waveform = Eigen::MatrixXf(2, buffers.padded_segment_samples);

        demucsonnx::istft(stft_buf, z_target, padded_waveform);

        // copy target waveform into all 4 dims of targets_out
        // summing with xt_3d in the process to merge the time and frequency branches
        for (int j = 0; j < 2; ++j)
        {
            for (int k = 0; k < buffers.segment_samples; ++k)
            {
                buffers.targets_out(source, j, k) = padded_waveform(j, buffers.pad + k) + xt_3d[source](j, k);
            }
        }
    }
}
