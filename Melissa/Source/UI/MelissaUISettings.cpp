#include "MelissaUISettings.h"

bool MelissaUISettings::isJa = false;

#if defined(JUCE_MAC)
bool MelissaUISettings::isMac = true;
String MelissaUISettings::japaneseFontName_ = "Hiragino Kaku Gothic Pro";
String MelissaUISettings::latinFontName_    = "San Francisco";

#else
bool MelissaUISettings::isMac = false;
String MelissaUISettings::japaneseFontName_ = "Meiryo UI";
String MelissaUISettings::latinFontName_    = "Tahoma";

#endif

String MelissaUISettings::fontName_ = "";
bool MelissaUISettings::useJapaneseFont_ = false;
