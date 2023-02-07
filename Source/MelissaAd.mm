//
//  MelissaAd.mm
//  Melissa
//
//  Copyright(c) 2023 Masaki Ono
//

#import "MelissaAd.h"
#import <GoogleMobileAds/GoogleMobileAds.h>

void MelissaAd::initialize()
{
    [GADMobileAds.sharedInstance startWithCompletionHandler:nil];
}
