#include "header.h"

void notifyConnection(){

    int notifySocketFd;
    struct sockaddr_in addr;

    notifySocketFd=Socket(AF_INET,SOCK_STREAM,0);//CREO LA SOCKET PER LA CONNESSIONE

    addr.sin_port=htons(SERVER_NOTIFY_PORT);
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr(serverIp);
    //SETTO LA SOCKET

    Connect(notifySocketFd, (struct sockaddr*)&addr,sizeof(addr));// MI CONNETTO

    //invio il mio id per notificarlo
    FullWrite(notifySocketFd,&myInfo,sizeof(PeerInfo));
    close(notifySocketFd);
}

PeerInfo getPeerInfo(int idPeer){

    int getInfoSocket, enable=1;
    struct sockaddr_in addr;
    PeerInfo peerInfo;

    getInfoSocket=Socket(AF_INET,SOCK_STREAM,0);//CREO LA SOCKET PER LA CONNESSIONE

    addr.sin_port=htons(SERVER_REQUEST_PORT);
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr(serverIp);
    //SETTO LA SOCKET

    Connect(getInfoSocket, (struct sockaddr*)&addr,sizeof(addr));// MI CONNETTO

    FullWrite(getInfoSocket, &idPeer, sizeof(int));
    //Richiedo le info del peer
    FullRead(getInfoSocket,&peerInfo,sizeof(PeerInfo));

    close(getInfoSocket);
    return peerInfo;//RITORNA LE INFO SE TROVATE DAL SERVER, ALTRIMENTI NULL

}
