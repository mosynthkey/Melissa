//
//  Copyright (C) 2015 Google, Inc.
//
//  AdMobAdDelegateViewController.m
//  APIDemo
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#ifdef ENABLE_MOBILEAD

#import <GoogleMobileAds/GoogleMobileAds.h>
#import "AdMobAdDelegateViewController.h"

/// Google AdMob - Ad Delegate
/// Demonstrates handling GADBannerViewDelegate ad request status and ad click lifecycle messages.
/// This is an AdMob example, so it uses a GADBannerView to show an AdMob ad. GADBannerViewDelegate
/// also works with GAMBannerView objects, so publishers displaying ads from
/// Google Ad Manager (GAM) can also use it with their banners.
/// To see this in action, use the GAMBannerView class instead of GADBannerView.
@interface AdMobAdDelegateViewController () <GADBannerViewDelegate>

/// The banner view.
@property(nonatomic, strong) GADBannerView *bannerView;

@end

@implementation AdMobAdDelegateViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.bannerView = [[GADBannerView alloc] initWithAdSize:GADAdSizeBanner];
    self.bannerView.delegate = self;
    self.bannerView.adUnitID = @"ca-app-pub-3940256099942544/2934735716";
    self.bannerView.rootViewController = self;
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    [self addBannerViewToView:self.bannerView];
    [self.bannerView loadRequest:[GADRequest request]];
}

- (void)addBannerViewToView:(UIView *)bannerView {
    bannerView.translatesAutoresizingMaskIntoConstraints = NO;
    [self.view addSubview:bannerView];
    [self.view addConstraints:@[
        [NSLayoutConstraint constraintWithItem:bannerView
                                     attribute:NSLayoutAttributeTop
                                     relatedBy:NSLayoutRelationEqual
                                        toItem:self.view.safeAreaLayoutGuide
                                     attribute:NSLayoutAttributeTop
                                    multiplier:1
                                      constant:0],
        [NSLayoutConstraint constraintWithItem:bannerView
                                     attribute:NSLayoutAttributeLeft
                                     relatedBy:NSLayoutRelationEqual
                                        toItem:self.view.safeAreaLayoutGuide
                                     attribute:NSLayoutAttributeLeft
                                    multiplier:1
                                      constant:0]
    ]];
}

#pragma mark - GADBannerViewDelegate

// Called when an ad request loaded an ad.
- (void)adViewDidReceiveAd:(GADBannerView *)bannerView {
    NSLog(@"%s", __PRETTY_FUNCTION__);
    [self addBannerViewToView:self.bannerView];
}

// Called when an ad request failed.
- (void)adView:(GADBannerView *)bannerView didFailToReceiveAdWithError:(NSError *)error {
    NSLog(@"%s: %@", __PRETTY_FUNCTION__, error.localizedDescription);
}

// Called just before presenting the user a full screen view, such as a browser, in response to
// clicking on an ad.
- (void)adViewWillPresentScreen:(GADBannerView *)bannerView {
    NSLog(@"%s", __PRETTY_FUNCTION__);
}

// Called just before dismissing a full screen view.
- (void)adViewWillDismissScreen:(GADBannerView *)bannerView {
    NSLog(@"%s", __PRETTY_FUNCTION__);
}

// Called just after dismissing a full screen view.
- (void)adViewDidDismissScreen:(GADBannerView *)bannerView {
    NSLog(@"%s", __PRETTY_FUNCTION__);
}

// Called just before the application will background or exit because the user clicked on an ad
// that will launch another application (such as the App Store).
- (void)adViewWillLeaveApplication:(GADBannerView *)bannerView {
    NSLog(@"%s", __PRETTY_FUNCTION__);
}

@end


@interface RewardAdViewController ()

@property(nonatomic, strong) GADRewardedAd *rewardedAd;

@end

@implementation RewardAdViewController
- (void)loadRewardedAd {
    GADRequest *request = [GADRequest request];
    [GADRewardedAd
     loadWithAdUnitID:@"ca-app-pub-3940256099942544/1712485313"
     request:request
     completionHandler:^(GADRewardedAd *ad, NSError *error) {
        if (error) {
            NSLog(@"Rewarded ad failed to load with error: %@", [error localizedDescription]);
            return;
        }
        self.rewardedAd = ad;
        NSLog(@"Rewarded ad loaded.");
    }];
}


/// Tells the delegate that the ad failed to present full screen content.
- (void)ad:(nonnull id<GADFullScreenPresentingAd>)ad
didFailToPresentFullScreenContentWithError:(nonnull NSError *)error {
    NSLog(@"Ad did fail to present full screen content.");
}

/// Tells the delegate that the ad presented full screen content.
- (void)adDidPresentFullScreenContent:(nonnull id<GADFullScreenPresentingAd>)ad {
    NSLog(@"Ad did present full screen content.");
}

/// Tells the delegate that the ad dismissed full screen content.
- (void)adDidDismissFullScreenContent:(nonnull id<GADFullScreenPresentingAd>)ad {
    NSLog(@"Ad did dismiss full screen content.");
}

- (void)show {
    if (self.rewardedAd) {
        [self.rewardedAd presentFromRootViewController:self
                              userDidEarnRewardHandler:^{
            GADAdReward *reward =
            self.rewardedAd.adReward;
            // TODO: Reward the user!
        }];
    } else {
        NSLog(@"Ad wasn't ready");
    }
}

@end

#endif
