//
// Created by Максим Воробей on 17.08.13.
// Copyright (c) 2013 n3b All rights reserved.
//


#import "CRESTResponse.h"

static void addHeaders(CFHTTPMessageRef response)
{
	CFHTTPMessageSetHeaderFieldValue(response, CFSTR("Connection"), CFSTR("close"));
	CFHTTPMessageSetHeaderFieldValue(response, CFSTR("Cache-Control"), CFSTR("no-cache"));
	CFHTTPMessageSetHeaderFieldValue(response, CFSTR("Content-Type"), CFSTR("text/html; charset=UTF-8"));
	// todo add more response types
}

CFDataRef CRESTResponseCopyDataOfBadRequest()
{
	CFHTTPMessageRef response = CFHTTPMessageCreateResponse(kCFAllocatorDefault, (CFIndex) 400, CFSTR("Bad Request"),
			kCFHTTPVersion1_1);

	addHeaders(response);

	CFDataRef data = CFHTTPMessageCopySerializedMessage(response);
	CFRelease(response);

	return data;
}

CFDataRef CRESTResponseCopyDataOfNotFound()
{
	CFHTTPMessageRef response = CFHTTPMessageCreateResponse(kCFAllocatorDefault, (CFIndex) 404, CFSTR("Not Found"),
			kCFHTTPVersion1_1);

	addHeaders(response);

	CFDataRef data = CFHTTPMessageCopySerializedMessage(response);
	CFRelease(response);

	return data;
}

CFDataRef CRESTResponseCopyDataOfOk(CFStringRef body)
{
	CFRetain(body);

	CFHTTPMessageRef response = CFHTTPMessageCreateResponse(kCFAllocatorDefault, (CFIndex) 200, CFSTR("OK"),
			kCFHTTPVersion1_1);

	addHeaders(response);

	CFDataRef bodyData = CFStringCreateExternalRepresentation(kCFAllocatorDefault, body, kCFStringEncodingUTF8, 0);
	CFHTTPMessageSetBody(response, bodyData);
	CFDataRef data = CFHTTPMessageCopySerializedMessage(response);

	CFRelease(response);
	CFRelease(bodyData);
	CFRelease(body);

	return data;
}
