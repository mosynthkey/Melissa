/*
  ==============================================================================

    split.h
    Created: 1 Jun 2020 3:13:58pm
    Author:  gvne

  ==============================================================================
*/

#pragma once

#include <map>
#include "spleeter/spleeter.h"

std::map<std::string, spleeter::Waveform>
Split(const spleeter::Waveform &waveform, spleeter::SeparationType type,
      std::error_code &err);
