/*
  ==============================================================================

    split.cpp
    Created: 1 Jun 2020 3:14:04pm
    Author:  gvne

  ==============================================================================
*/

#include "split.h"

std::map<std::string, spleeter::Waveform> SplitTwo(const spleeter::Waveform &waveform, std::error_code &err) {
  spleeter::Waveform vocals, accompaniment;
  spleeter::Split(waveform, &vocals, &accompaniment, err);
  if (err) {
    return {};
  }
  return {
    {"vocals", vocals},
    {"accompaniment", accompaniment}
  };
}

std::map<std::string, spleeter::Waveform> SplitFour(const spleeter::Waveform &waveform, std::error_code &err) {
  spleeter::Waveform vocals, drums, bass, other;
  spleeter::Split(waveform, &vocals, &drums, &bass, &other, err);
  if (err) {
    return {};
  }
  return {
    {"vocals", vocals},
    {"drums", drums},
    {"bass", bass},
    {"other", other}
  };
}

std::map<std::string, spleeter::Waveform> SplitFive(const spleeter::Waveform &waveform, std::error_code &err) {
  spleeter::Waveform vocals, drums, bass, piano, other;
  spleeter::Split(waveform, &vocals, &drums, &bass, &piano, &other, err);
  if (err) {
    return {};
  }
  return {
    {"vocals", vocals},
    {"drums", drums},
    {"bass", bass},
    {"piano", piano},
    {"other", other}
  };
}

std::map<std::string, spleeter::Waveform>
Split(const spleeter::Waveform &waveform, spleeter::SeparationType type,
      std::error_code &err) {
  switch (type) {
  case spleeter::TwoStems:
    return SplitTwo(waveform, err);
  case spleeter::FourStems:
    return SplitFour(waveform, err);
  case spleeter::FiveStems:
    return SplitFive(waveform, err);
  default:
    err = std::make_error_code(std::errc::not_supported);
    return {};
  }
}
