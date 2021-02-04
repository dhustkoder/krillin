#ifndef KRLNET_H_
#define KRLNET_H_
#include "types.h"



void krlnet_init(void);
void krlnet_term(void);
void krlnet_socket_init(krln_socket_t* sock, const char* url, uint16_t port);
void krlnet_socket_send(krln_socket_t* sock, const char* data);
void krlnet_socket_recv(krln_socket_t* sock);
void krlnet_socket_term(krln_socket_t* sock);





#endif