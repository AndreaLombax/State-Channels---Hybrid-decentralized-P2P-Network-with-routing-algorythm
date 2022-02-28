#include "headerServer.h"

int Socket(int domain,int type,int protocol){
    int sockfd;
    if ((sockfd=socket(domain,type,protocol))<0)
    {
        printf("Errore apertura Socket\n");
        exit(-1);
    }
    return sockfd;
}

int Connect(int sockfd, const struct sockaddr *addr,socklen_t addrlen)
{
    if (connect(sockfd,addr,addrlen)<0)
    {
        printf("Errore in Connect\n");
        return -1;
    }
}

void Bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen)
{
    if (bind(sockfd,addr,addrlen)<0)
    {
        printf("Errore in Bind\n");
        exit(-1);
    }
}

void Listen(int sockfd, int backlog)
{
    if (listen(sockfd,backlog)<0)
    {
        printf("Errore in Listen\n");
        exit(-1);
    }
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int clientfd;
    if ((clientfd=accept(sockfd,addr,addrlen))<0)
    {
        printf("Errore in Accept\n");
        exit(-1);
    }
    return clientfd;
}

ssize_t FullWrite(int fd, const void *buf, size_t count)
{
    size_t nleft;
    ssize_t nwritten;
    nleft = count;
    while (nleft > 0)
    {
        if ((nwritten = write(fd, buf, nleft)) < 0)
        {
            printf("Errore in write\n");
            if (errno == EINTR) /* if interrupted by system call */
                continue;
            else 
                exit(nwritten); /* otherwise exit with error */
        }
        nleft -= nwritten; /* set left to write */
        buf +=nwritten; /* set pointer */
    }
    return (count-nleft);
}

ssize_t FullRead(int fd, void *buf, size_t count)
{
    size_t nleft;
    ssize_t nread;
    nleft = count;
    while (nleft > 0) /* repeat until no left */ 
    { 
        if ( (nread = read(fd, buf, nleft)) < 0) 
        {
            if (errno == EINTR) /* if interrupted by system call */
                continue;
            else
                exit(nread);
        } 
        else if (nread == 0)  /* EOF */
            break;
        nleft-=nread; /* set left to read */
        buf+=nread; /* set pointer */
    }
    buf=0;
    return (count-nleft);
}