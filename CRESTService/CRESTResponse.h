//
// Created by Максим Воробей on 17.08.13.
// Copyright (c) 2013 n3b All rights reserved.
//


#import <CoreServices/CoreServices.h>

CFDataRef CRESTResponseCopyDataOfBadRequest();

CFDataRef CRESTResponseCopyDataOfNotFound();

CFDataRef CRESTResponseCopyDataOfOk(CFStringRef body);
