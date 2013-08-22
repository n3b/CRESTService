//
// Created by Максим Воробей on 11.08.13.
// Copyright (c) 2013 n3b All rights reserved.
//

#import <sys/socket.h>
#import <netinet/in.h>
#import <stdio.h>
#import <dispatch/dispatch.h>
#import <stdlib.h>

#import <CoreServices/CoreServices.h>
#import "CRESTRequest.h"
#import "CRESTResponse.h"

typedef CFStringRef (^CRESTCallback)(CFDictionaryRef params);

CFSocketError CRESTServiceStartListenOnPort(UInt16 port);

CFSocketError CRESTServiceStopListen();

void CRESTServiceAddCallback(CFStringRef path, CRESTCallback cb);
