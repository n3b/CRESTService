//
// Created by Максим Воробей on 17.08.13.
// Copyright (c) 2013 n3b All rights reserved.
//


#import <CoreServices/CoreServices.h>

struct CRESTRequest
{
	CFStringRef method;
	CFStringRef path;
	CFDictionaryRef params;
};
typedef struct CRESTRequest * CRESTRequestRef;

CRESTRequestRef CRESTRequestCreateWithBytes(UInt8 const *bytes, ssize_t len);
void CRESTRequestDealloc(CRESTRequestRef request);
