//
//  MainViewController.m
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/1.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import "MainViewController.h"
#import "LivingViewController.h"
#import "EncoderViewController.h"
#import "AudioViewController.h"

@interface MainViewController ()

@end

@implementation MainViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view from its nib.
}

- (IBAction)startLiving:(UIButton *)sender {
    [self.navigationController pushViewController:[[LivingViewController alloc] initWithNibName:@"LivingViewController" bundle:nil] animated:YES];
}

- (IBAction)startEncoder:(UIButton *)sender {
    [self.navigationController pushViewController:[[EncoderViewController alloc] initWithNibName:@"EncoderViewController" bundle:nil] animated:YES];
}
- (IBAction)startAudioEncoder:(UIButton *)sender {
    [self.navigationController pushViewController:[[AudioViewController alloc] initWithNibName:@"AudioViewController" bundle:nil] animated:YES];
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
