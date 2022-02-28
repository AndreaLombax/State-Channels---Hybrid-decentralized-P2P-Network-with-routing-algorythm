#ifndef HEADERSERVER_H_INCLUDED
#define HEADERSERVER_H_INCLUDED
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <pthread.h>
#include <malloc.h>
#include <stdbool.h>
#include <signal.h>

#define registrationPort 1500
#define searchPort 2520
#define sizeQueue 1000

typedef struct PeerInfo
{
    int idPeer;
    char ipAddress[16];
    int channelPort;//PORTA CHE OFFRE IL SERVIZIO DI CREAZIONE CANALE
    int updatePort;//PORTA CHE OFFRE IL SERVIZIO DI AGGIORNAMENTI DI TABELLE DI ROUTING
    int transactionPort;
}PeerInfo;


//FUNZIONI WRAPPER
int Socket(int domain,int type,int protocol);
int Connect(int sockfd, const struct sockaddr *addr,socklen_t addrlen);
void Listen(int sockfd, int backlog);
void Bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen);
int Accept(int sockfd, struct sockaddr *clientaddr, socklen_t *addr_dim);
ssize_t FullRead(int fd, void *buf, size_t count);
ssize_t FullWrite(int fd, const void *buf, size_t count);
//------------------------------------------------//


void searchPeer(PeerInfo* peerInfo);
void addInfo(PeerInfo* peerInfo);


#endif // HEADERSERVER_H_INCLUDED