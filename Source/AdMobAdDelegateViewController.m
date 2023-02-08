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
    self.bannerView = [[GADBannerView alloc] init];
    self.bannerView.delegate = self;
    [self addBannerViewToView:self.bannerView];
    
    self.bannerView.adUnitID = @"ca-app-pub-3940256099942544/2934735716";
    self.bannerView.rootViewController = self;
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    // Note loadBannerAd is called in viewDidAppear as this is the first time that
    // the safe area is known. If safe area is not a concern (e.g., your app is
    // locked in portrait mode), the banner can be loaded in viewWillAppear.
    [self loadBannerAd];
}

- (void)viewWillTransitionToSize:(CGSize)size
       withTransitionCoordinator:(id)coordinator {
    [super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
    [coordinator animateAlongsideTransition:^(id
                                              _Nonnull context) {
        [self loadBannerAd];
    } completion:nil];
}

- (void)loadBannerAd {
    // Step 2 - Determine the view width to use for the ad width.
    CGRect frame = self.view.frame;
    // Here safe area is taken into account, hence the view frame is used after
    // the view has been laid out.
    if (@available(iOS 11.0, *)) {
        frame = UIEdgeInsetsInsetRect(self.view.frame, self.view.safeAreaInsets);
    }
    CGFloat viewWidth = frame.size.width;
    
    // Step 3 - Get Adaptive GADAdSize and set the ad view.
    // Here the current interface orientation is used. If the ad is being
    // preloaded for a future orientation change or different orientation, the
    // function for the relevant orientation should be used.
    self.bannerView.adSize = GADCurrentOrientationAnchoredAdaptiveBannerAdSizeWithWidth(viewWidth);
    
    // Step 4 - Create an ad request and load the adaptive banner ad.
    GADRequest *request = [GADRequest request];
    [self.bannerView loadRequest:request];
}

- (void)addBannerViewToView:(UIView *)bannerView {
    bannerView.translatesAutoresizingMaskIntoConstraints = NO;
    [self.view addSubview:bannerView];
    [self.view addConstraints:@[
        [NSLayoutConstraint constraintWithItem:bannerView
                                     attribute:NSLayoutAttributeBottom
                                     relatedBy:NSLayoutRelationEqual
                                        toItem:self.view.safeAreaLayoutGuide
                                     attribute:NSLayoutAttributeBottom
                                    multiplier:1
                                      constant:0],
        [NSLayoutConstraint constraintWithItem:bannerView
                                     attribute:NSLayoutAttributeCenterX
                                     relatedBy:NSLayoutRelationEqual
                                        toItem:self.view
                                     attribute:NSLayoutAttributeCenterX
                                    multiplier:1
                                      constant:0]
    ]];
}

#pragma mark - GADBannerViewDelegate

// Called when an ad request loaded an ad.
- (void)adViewDidReceiveAd:(GADBannerView *)bannerView {
    NSLog(@"%s", __PRETTY_FUNCTION__);
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
