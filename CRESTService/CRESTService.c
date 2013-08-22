//
// Created by Максим Воробей on 11.08.13.
// Copyright (c) 2013 n3b All rights reserved.
//

#import "CRESTService.h"

typedef struct {
	struct sockaddr_in addr;
	int fd;
	char buffer[8196];
	dispatch_source_t source;
} SocketClient;
typedef SocketClient * SocketClientRef;

static CFMutableDictionaryRef callbacks;
static SocketClientRef listener;
static dispatch_queue_t socketReadQueue;
static dispatch_queue_t socketAcceptQueue;

static void closeSocketClient(SocketClientRef client)
{
	dispatch_resume(client->source);
	dispatch_source_cancel(client->source);
	dispatch_release(client->source);
	close(client->fd);
	free(client);
}

static CFDataRef handleRequest(CRESTRequestRef request)
{
	CFDataRef responseData;
	CRESTCallback cb;

	// request is empty
	if( ! request ) {
		responseData = CRESTResponseCopyDataOfBadRequest();
	}

	// no callback
	else if( ! ( callbacks && CFDictionaryContainsKey(callbacks, request->path) ) ) {
		responseData = CRESTResponseCopyDataOfNotFound();
	}

	// callback
	else if( ( cb = CFDictionaryGetValue(callbacks, request->path) ) ){
		CFStringRef cbResult = CFStringCreateCopy(kCFAllocatorDefault, cb(request->params));
		responseData = CRESTResponseCopyDataOfOk(cbResult);
		CFRelease(cbResult);
	}

	// wrong callback
	else {
		responseData = CFDataCreate(kCFAllocatorDefault, 0, 0);
	}

	return responseData;
}

static void onSocketDataReceived(SocketClientRef client, size_t available)
{
	if( ! listener ) return;

	assert( dispatch_get_current_queue() == socketReadQueue );
	dispatch_suspend(client->source);

	CRESTRequestRef request = NULL;
	size_t maxRead = sizeof(client->buffer) - 1;

	if( available > maxRead ) available = maxRead;

	if( available > 14 )
	{
		int i = 0;
		char c;
		while( i < available && recv(client->fd, &c, 1, 0) > 0 && c > 0 )
		{
			client->buffer[i] = c;
			i++;
		}

		request = CRESTRequestCreateWithBytes((UInt8 const *)&client->buffer, i);
	}

	CFDataRef responseData = handleRequest(request);
	ssize_t wd = send(client->fd, (void const *)CFDataGetBytePtr(responseData),
			(size_t)CFDataGetLength(responseData), 0);
// todo
#pragma unused(wd)

	CFRelease(responseData);
	CRESTRequestDealloc(request);
	closeSocketClient(client);
}

static void onSocketAccept()
{
	if( ! listener ) return;
	assert( dispatch_get_current_queue() == socketAcceptQueue );

	SocketClientRef client = calloc(1, sizeof(SocketClient));
	assert(client);
	socklen_t len = sizeof(client->addr);
	client->fd = accept(listener->fd, (struct sockaddr *)&client->addr, &len);

	client->source = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, (uintptr_t) client->fd, 0, socketReadQueue);
	dispatch_source_set_event_handler(client->source, ^{
		onSocketDataReceived(client, dispatch_source_get_data(client->source)); });

	//static int request = 0;
	//printf("request %d on fd %d\n", request++, client->fd);
	dispatch_resume(client->source);
}

CFSocketError CRESTServiceStartListenOnPort(UInt16 port)
{
	//todo allow listeners on other ports
	assert( ! listener );

	CFSocketRef socket = CFSocketCreate(NULL, PF_INET, SOCK_STREAM, IPPROTO_IP, 0, NULL, NULL);
	if( ! socket ) return kCFSocketError;

	listener = calloc(1, sizeof(SocketClient));
	assert(listener);
	listener->addr.sin_port = htons(port);
	listener->fd = CFSocketGetNative(socket);

	CFDataRef address = CFDataCreate(NULL,(UInt8 *)&listener->addr,sizeof(listener->addr));
	CFSocketError err = CFSocketSetAddress(socket, address);
	CFRelease(address);
	CFRelease(socket);

	if( kCFSocketSuccess == err ) {

		socketReadQueue = dispatch_queue_create("ru.n3b.SocketReadQueue", NULL);
		socketAcceptQueue = dispatch_queue_create("ru.n3b.SocketAcceptQueue", NULL);

		listener->source = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, (uintptr_t)listener->fd, 0,
				socketAcceptQueue);
		dispatch_source_set_event_handler(listener->source, ^{ onSocketAccept(); });
		dispatch_resume(listener->source);

		return kCFSocketSuccess;

	} else {

		close(listener->fd);
		free(listener);

		return kCFSocketError;
	}
}

CFSocketError CRESTServiceStopListen()
{
	assert( listener );
	dispatch_suspend(listener->source);
	closeSocketClient(listener);

	dispatch_release(socketReadQueue);
	dispatch_release(socketAcceptQueue);

	CFRelease(callbacks);
	listener = NULL;
	callbacks = NULL;
	
	return kCFSocketSuccess;
}

void CRESTServiceAddCallback(CFStringRef path, CRESTCallback cb)
{
	if( ! callbacks )
	{
		callbacks = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFCopyStringDictionaryKeyCallBacks,
			&kCFTypeDictionaryValueCallBacks);
	}

	CFDictionaryAddValue(callbacks, path, cb);
}
