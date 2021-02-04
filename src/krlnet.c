#include <ws2tcpip.h>
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "krlnet.h"


static bool sock_is_filled(SOCKET s)
{
	struct timeval tv = { .tv_sec = 0, .tv_usec = 8000 };
	fd_set set;
	FD_ZERO(&set);
	FD_SET(s, &set);
	
	int retval = select(
		0,
		&set,
		NULL,
		NULL,
		&tv
	);
	
	if (retval != SOCKET_ERROR && retval != 0)
		return true;
	
	return false;
}



void krlnet_init(void)
{
	int ret;
	WSADATA wsa_data;
	
	ret = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	assert(ret == 0);
}

void krlnet_term(void)
{
	WSACleanup();
}

void krlnet_socket_init(krln_socket_t* sock, const char* url, uint16_t port)
{
	memset(sock, 0, sizeof(*sock));
	
	int ret;
	const char* ip;
	struct sockaddr_in addr;
	struct hostent* host;
	SOCKET handle;
	
	handle = socket(
	  AF_INET,
	  SOCK_STREAM,
	  IPPROTO_TCP
	);
	
	assert(handle != INVALID_SOCKET);
	
	host = gethostbyname(url);
	
	assert(host != NULL);
	
	ip = inet_ntoa(*(struct in_addr *)*host->h_addr_list);
	
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	InetPton(AF_INET, ip, &addr.sin_addr);
	
	ret = connect(handle, (struct sockaddr*)&addr, sizeof addr);
	assert(ret == 0);
	
	sock->handle = handle;
	sock->recv_reserved = 2048;
	sock->recv_data = (uint8_t*) malloc(sock->recv_reserved);
	
}

void krlnet_socket_send(krln_socket_t* sock, const char* data)
{
	send(
		sock->handle,
		data,
		strlen(data),
		0
	);
}

void krlnet_socket_recv(krln_socket_t* sock)
{
	int retval;
	
	sock->recv_size = 0;
	
	if (!sock_is_filled(sock->handle))
		return;
	
	retval = recv(
		(SOCKET)sock->handle,
		(char*)sock->recv_data,
		1024,
		0
	);
	
	if (retval == SOCKET_ERROR) {
		printf("SOCKET ERROR\n");
		return;
	}

	sock->recv_size = retval;
	sock->recv_data[sock->recv_size] = '\0';
}


void krlnet_socket_term(krln_socket_t* sock)
{
	free(sock->recv_data);
	closesocket((SOCKET)sock->handle);
}



