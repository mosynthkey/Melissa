/*
  ==============================================================================

    utils.h
    Created: 1 Jun 2020 2:34:39pm
    Author:  gvne

  ==============================================================================
*/

#pragma once

#include <Eigen/Core>

Eigen::MatrixXf Stereo(const Eigen::MatrixXf &input);

Eigen::MatrixXf Resample(const Eigen::MatrixXf &input,
                         double original_sampling_rate,
                         double target_sampling_rate);
