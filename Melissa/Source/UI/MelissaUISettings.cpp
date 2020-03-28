#include "MelissaUISettings.h"

bool MelissaUISettings::isJa = false;

#if defined(JUCE_MAC)
bool MelissaUISettings::isMac = true;
#else
bool MelissaUISettings::isMac = false;
#endif

