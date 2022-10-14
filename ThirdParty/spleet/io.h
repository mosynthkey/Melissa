/*
  ==============================================================================

    io.h
    Created: 1 Jun 2020 2:33:13pm
    Author:  gvne

  ==============================================================================
*/

#pragma once

#include "Eigen/Core"

void Read(const std::string &path, Eigen::MatrixXf *data, double *sampling_rate,
          std::error_code &err);

void Write(const Eigen::MatrixXf &data, double sampling_rate,
           const std::string &destination, std::error_code &err);
