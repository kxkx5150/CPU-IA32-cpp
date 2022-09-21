#ifndef HALFIX_NET_H
#define HALFIX_NET_H
int  net_init(char *netarg);
int  net_send(void *req, int reqlen);
void net_poll(void (*)(void *data, int len));
#endif