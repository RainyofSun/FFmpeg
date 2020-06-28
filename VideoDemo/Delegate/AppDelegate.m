//
//  AppDelegate.m
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/1.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import "AppDelegate.h"
#import "MainViewController.h"

@interface AppDelegate ()

@end

@implementation AppDelegate


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    self.window = [[UIWindow alloc] init];
    MainViewController *mainVC = [[MainViewController alloc] initWithNibName:@"MainViewController" bundle:nil];
    self.window.rootViewController = [[UINavigationController alloc] initWithRootViewController:mainVC];
    [self.window makeKeyAndVisible];
    
    return YES;
}


@end
