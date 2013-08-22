//
//  main.c
//  CRESTService
//
//  Created by Максим Воробей on 15.08.13.
//  Copyright (c) 2013 Максим Воробей. All rights reserved.
//

#import "CRESTService.h"

int main(int argc, const char * argv[])
{
	CRESTServiceAddCallback(CFSTR("/test/test"), ^(CFDictionaryRef params){

		CFStringRef val = CFDictionaryGetValue(params, CFSTR("key"));

		if( val ) {
			return val;
		}

		return CFSTR("");
	});

	if( kCFSocketSuccess == CRESTServiceStartListenOnPort(8080) ) {

		sleep(600);

		CRESTServiceStopListen();

	} else {
		return 1;
	}

    return 0;
}

