//
// Created by Максим Воробей on 17.08.13.
// Copyright (c) 2013 n3b All rights reserved.
//


#import "CRESTRequest.h"

static const CFStringRef amp = CFSTR("&");
static const CFStringRef eq = CFSTR("=");
static const CFStringRef pl = CFSTR("+");
static const CFStringRef sp = CFSTR(" ");
static const CFStringRef emp = CFSTR("");

static void parseParams(CFMutableDictionaryRef params, CFStringRef query)
{
	assert( params && query );

	CFArrayRef parts = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, query, amp);
	int i = 0;

	while( i < CFArrayGetCount(parts) )
	{
		CFArrayRef components = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault,
				CFArrayGetValueAtIndex(parts, i), eq);

		// only if k & v present, and len of k > 0
		if( 2 == CFArrayGetCount(components) && CFStringGetLength(CFArrayGetValueAtIndex(components, 0)) )
		{
			// retain count == 2
			CFStringRef val = CFURLCreateStringByReplacingPercentEscapesUsingEncoding(kCFAllocatorDefault,
					CFArrayGetValueAtIndex(components, 1), emp, kCFStringEncodingUTF8);
			CFDictionarySetValue(params, CFArrayGetValueAtIndex(components, 0), val);
			CFRelease(val);
		}

		CFRelease(components);
		i++;
	}

	CFRelease(parts);
}

CRESTRequestRef CRESTRequestCreateWithBytes(UInt8 const *bytes, ssize_t len)
{
	CFURLRef url;
	CRESTRequestRef request;
	CFHTTPMessageRef message;

	message	= CFHTTPMessageCreateEmpty(kCFAllocatorDefault, true);
	request = calloc(1, sizeof(struct CRESTRequest));
	assert(request);
	bool isAppended = CFHTTPMessageAppendBytes(message, bytes, (CFIndex) len);
	bool isComplete = CFHTTPMessageIsHeaderComplete(message);

	if( ! ( isAppended && isComplete ) )
	{
		CFRelease(message);
		CRESTRequestDealloc(request);
		return NULL;
	}

	url = CFHTTPMessageCopyRequestURL(message);

	CFMutableDictionaryRef params = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
			&kCFTypeDictionaryValueCallBacks);

	CFStringRef query = CFURLCopyQueryString(url, NULL);
	if( query )
	{
		parseParams(params, query);
		CFRelease(query);
	}

	CFDataRef bodyData = CFHTTPMessageCopyBody(message);
	if( CFDataGetLength(bodyData) )
	{
		CFStringRef bodyString = CFStringCreateFromExternalRepresentation(kCFAllocatorDefault, bodyData,
				kCFStringEncodingUTF8);
		parseParams(params, bodyString);
		CFRelease(bodyString);
	}
	CFRelease(bodyData);

	request->method = CFHTTPMessageCopyRequestMethod(message);
	request->path = CFURLCopyPath(url);
	request->params = (CFDictionaryRef) params;

	CFRelease(url);
	CFRelease(message);

	return request;
}

void CRESTRequestDealloc(CRESTRequestRef request)
{
	if( ! request ) return;

	if(request->method) CFRelease(request->method);
	if(request->path) CFRelease(request->path);
	if(request->params) CFRelease(request->params);
	free(request);
}
