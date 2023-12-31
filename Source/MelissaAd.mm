//
//  MelissaAd.mm
//  Melissa
//
//  Copyright(c) 2023 Masaki Ono
//

#ifdef ENABLE_MOBILEAD
#import "MelissaAd.h"
#import <GoogleMobileAds/GoogleMobileAds.h>

void MelissaAd::initialize()
{
    [GADMobileAds.sharedInstance startWithCompletionHandler:nil];
    GADMobileAds.sharedInstance.requestConfiguration.testDeviceIdentifiers = @[ GADSimulatorID ];
}

#endif
