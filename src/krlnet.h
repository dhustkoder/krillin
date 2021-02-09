#ifndef KRLNET_H_
#define KRLNET_H_
#include "krl.h"




void krlnet_init(void);
void krlnet_term(void);
void krlnet_connect(const char* url, uint16_t port);
void krlnet_write(const char* fmt, ...);
size_t krlnet_readline(char* buffer, size_t maxlen);


#endif