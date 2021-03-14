#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "krl.h"

#define CSL_STATIC
#define CSL_SOCKET_IMPLEMENTATION
#include "csl_socket.h"


#define MAX_STREAM_LEN      (4096)


typedef struct stream {
	char data[MAX_STREAM_LEN + 1];
	size_t len;
} stream_t;

static FILE* nul_file;
static Socket sock;
static stream_t out_stream;
static stream_t in_stream;


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
	csl_socket_send(sock, s->data, s->len, CSL_IO_OPT_WAIT);
	stream_rm_data(s, s->len);
}

static void stream_recv(stream_t* s)
{
	int cnt = MAX_STREAM_LEN - s->len;
	if (cnt == 0) {
		printf("STREAM FULL\n");
		return;
	}
	
	cnt = csl_socket_recv(sock, s->data + s->len, cnt, CSL_IO_OPT_DONTWAIT);
	
	if (cnt == CSL_SOCKET_ERROR) {
		printf("RECV SOCKET ERROR\n");
		return;
	}
	
	s->len += cnt;
	s->data[s->len] = '\0';
}









void krlnet_init(void)
{
	int ret;
	ret = csl_socket_init();
	assert(ret == 0);

	nul_file = fopen("nul", "w");
	assert(nul_file != NULL);
}

void krlnet_term(void)
{
	csl_socket_close(sock);
	fclose(nul_file);
	csl_socket_term();
}

void krlnet_connect(const char* url, uint16_t port)
{
	int ret;
	
	sock = csl_socket_open(CSL_PROTOCOL_TCP);	
	assert(sock != CSL_INVALID_SOCKET);
	
	ret = csl_socket_connect_hostname(sock, url, port);	
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

