#include <ws2tcpip.h>
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "krl.h"

#define MAX_SOCKET_PKT_SIZE (2048)
#define MAX_STREAM_LEN      (4096)


typedef struct stream {
	char data[MAX_STREAM_LEN + 1];
	size_t len;
} stream_t;

static FILE* nul_file;
static SOCKET sock;
static stream_t out_stream;
static stream_t in_stream;


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


static void stream_rm_data(stream_t* s, size_t count)
{
	ARRAY_RM_MEMMOVE(s->data, s->len, 0, count);
	s->len -= count;
	s->data[s->len] = '\0';
}

static void stream_vprintf(stream_t* s, const char* fmt, va_list vl)
{
	int count = vfprintf(nul_file, fmt, vl);
	if ((s->len + count) > MAX_STREAM_LEN) {
		printf("FMT DOEST NOT FIT STREAM := ");
		vprintf(fmt, vl);
		printf("\n");
		return;
	}
	vsprintf(s->data + s->len, fmt, vl);
	s->len += count;
	s->data[s->len] = '\0';
}

static size_t stream_read(stream_t* s, char* dest, size_t count)
{
	if (s->len == 0)
		return 0;
	
	count = s->len > count ? count : s->len;
	memcpy(dest, s->data, count);
	dest[count] = '\0';
	stream_rm_data(s, count);
	
	return count;
}

static size_t stream_readline(stream_t* s, char* dest, size_t maxlen)
{
	if (s->len == 0)
		return 0;
	
	const char* nl = strchr(s->data, '\n');
	if (nl == NULL)
		return 0;
	
	++nl; // lets eat the nl
	size_t read_count = nl - s->data;
	
	if (read_count > maxlen) {
		// discard the too long line
		stream_rm_data(s, read_count);
		return 0;
	}
	
	return stream_read(s, dest, read_count);
}

static void stream_send(stream_t* s)
{
	while (s->len > 0) {
		int cnt = s->len > MAX_SOCKET_PKT_SIZE ? MAX_SOCKET_PKT_SIZE : s->len;
		cnt = send(sock, s->data, cnt, 0);
		if (cnt == SOCKET_ERROR) {
			printf("SEND SOCKET ERROR\n");
			return;
		}
		stream_rm_data(s, cnt);
	}
}

static void stream_recv(stream_t* s)
{
	if (!sock_is_filled(sock))
		return;
	
	int cnt = MAX_STREAM_LEN - s->len;
	cnt = cnt > MAX_SOCKET_PKT_SIZE ? MAX_SOCKET_PKT_SIZE : cnt;
	if (cnt == 0) {
		printf("STREAM FULL\n");
		return;
	}
	
	cnt = recv(sock, s->data + s->len, cnt, 0);
	
	if (cnt == SOCKET_ERROR) {
		printf("RECV SOCKET ERROR\n");
		return;
	}
	
	s->len += cnt;
	s->data[s->len] = '\0';
}









void krlnet_init(void)
{
	int ret;
	WSADATA wsa_data;
	
	ret = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	assert(ret == 0);

	nul_file = fopen("nul", "w");
	assert(nul_file != NULL);
}

void krlnet_term(void)
{
	closesocket(sock);
	fclose(nul_file);
	WSACleanup();
}

void krlnet_connect(const char* url, uint16_t port)
{
	const char* ip;
	struct sockaddr_in addr;
	struct hostent* host;
	int ret;
	
	sock = socket(
	  AF_INET,
	  SOCK_STREAM,
	  IPPROTO_TCP
	);
	
	assert(sock != INVALID_SOCKET);
	
	host = gethostbyname(url);
	
	assert(host != NULL);
	
	ip = inet_ntoa(*(struct in_addr *)*host->h_addr_list);
	
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	InetPton(AF_INET, ip, &addr.sin_addr);
	
	ret = connect(sock, (struct sockaddr*)&addr, sizeof addr);
	assert(ret == 0);
}

void krlnet_write(const char* fmt, ...)
{
	va_list vl;
	va_start(vl, fmt);
	stream_vprintf(&out_stream, fmt, vl);
	va_end(vl);
	
	stream_send(&out_stream);
}

size_t krlnet_readline(char* buffer, size_t maxlen)
{
	stream_recv(&in_stream);
	
	if (in_stream.len == 0)
		return 0;

	size_t read = stream_readline(&in_stream, buffer, maxlen);
	
	return read;
}
