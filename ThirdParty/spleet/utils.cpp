/*
  ==============================================================================

    utils.cpp
    Created: 1 Jun 2020 2:34:45pm
    Author:  gvne

  ==============================================================================
*/

#include "utils.h"
#include <Eigen/Core>
#include <JuceHeader.h>

Eigen::MatrixXf Resample(const Eigen::MatrixXf &source, double original,
                         double desired) {
  if (desired == original) {
    return source;
  }

  LagrangeInterpolator interpolator;
  auto speed_ratio = desired / original;

  auto source_frame_count = source.cols();
  auto desired_frame_count =
      static_cast<int>(ceil(source_frame_count * speed_ratio));
  Eigen::MatrixXf output(source.rows(), desired_frame_count);

  Eigen::VectorXf input_channel(source.cols());
  Eigen::VectorXf output_channel(output.cols());
  for (auto channel_idx = 0; channel_idx < source.rows(); channel_idx++) {
    input_channel.array() = source.row(channel_idx).array();
    auto interpolator_ratio = 1.0 / speed_ratio;
    interpolator.process(interpolator_ratio, input_channel.data(),
                         output_channel.data(), output_channel.size());
    output.row(channel_idx).array() = output_channel.array();
    interpolator.reset();
  }

  return output;
}

Eigen::MatrixXf Stereo(const Eigen::MatrixXf &data) {
  if (data.rows() == 2) {
    return data;
  }
  // make the mean over rows
  auto mean = data.array().colwise().sum() / data.rows();

  // duplicate it twice to get stereo
  Eigen::MatrixXf output(2, data.cols());
  output.row(0) = mean;
  output.row(1) = mean;
  return output;
}
