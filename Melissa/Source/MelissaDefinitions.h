//
//  MelissaDefinitions.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

static constexpr int kPitchMin = -24;
static constexpr int kPitchMax = 24;

static constexpr int kSpeedMin = 20;
static constexpr int kSpeedMax = 200;
    
static constexpr int kEqFreqMin = 20;
static constexpr int kEqFreqMax = 20000;
    
static constexpr int kEqGainMin = -24;
static constexpr int kEqGainMax = 24;
    
static constexpr float kEqQMin = 0.1;
static constexpr float kEqQMax = 2;

static constexpr int kBpmMin = 45;
static constexpr int kBpmMax = 300;
static constexpr int kBpmShouldMeasure = kBpmMin - 1;
static constexpr int kBpmMeasureFailed = kBpmShouldMeasure - 1;

static constexpr int kAccentMax = 16;

static constexpr float kMusicVolumeMax = 2.f;

enum { kNumOfSpeedPresets = 11 };
extern const int speedPresets[kNumOfSpeedPresets];
