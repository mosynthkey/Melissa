//
//  MelissaUISettings.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaUISettings.h"

bool MelissaUISettings::isJa = false;

#if defined(JUCE_MAC)
bool MelissaUISettings::isMac = true;
String MelissaUISettings::japaneseFontName_ = "YuGothic";
String MelissaUISettings::latinFontName_    = "San Francisco";
#elif defined(JUCE_LINUX)
bool MelissaUISettings::isMac = false;
String MelissaUISettings::japaneseFontName_ = "IPAGothic";
String MelissaUISettings::latinFontName_    = "Verdana";

#else
bool MelissaUISettings::isMac = false;
String MelissaUISettings::japaneseFontName_ = "Meiryo UI";
String MelissaUISettings::latinFontName_    = "Tahoma";

#endif

String MelissaUISettings::fontName_ = "";
bool MelissaUISettings::useJapaneseFont_ = false;
