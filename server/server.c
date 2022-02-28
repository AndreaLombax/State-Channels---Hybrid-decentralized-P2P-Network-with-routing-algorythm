#include "headerServer.h"

void signal_handler(int signal); 

void main (int argc, char *argv[])
{

    signal(SIGINT,signal_handler);
    signal(SIGTSTP,signal_handler);
    int listenFdRegisterPeer, listenFdSearchPeer, enable1=1, enable2=1,maxFd,clientFd,connectionNotify;
    struct sockaddr_in serverAddrRegister, serverAddrSearch, clientAddr;
    PeerInfo peerInfo;
    fd_set readSet;

    socklen_t sizeClientAddr=sizeof(clientAddr);
    
    //SETTO LA SOCKET PER L'ASCOLTO DELLE NUOVE CONNESSIONI
    listenFdRegisterPeer=Socket(AF_INET,SOCK_STREAM,0);
    serverAddrRegister.sin_family=AF_INET;
    serverAddrRegister.sin_port=htons(registrationPort);
    serverAddrRegister.sin_addr.s_addr=htonl(INADDR_ANY);

    setsockopt(listenFdRegisterPeer, SOL_SOCKET, SO_REUSEPORT, &enable1, sizeof(int));
    Bind(listenFdRegisterPeer,(struct sockaddr*)&serverAddrRegister,sizeof(serverAddrRegister));
    Listen(listenFdRegisterPeer,sizeQueue);    
    //------------------------------------------------//

    //SETTO LA SOCKET PER LE RICHIESTE DI RICERCA DI INFO
    listenFdSearchPeer=Socket(AF_INET,SOCK_STREAM,0);
    serverAddrSearch.sin_port=htons(searchPort);
    serverAddrSearch.sin_family=AF_INET;
    serverAddrSearch.sin_addr.s_addr=htonl(INADDR_ANY);
    setsockopt(listenFdSearchPeer, SOL_SOCKET, SO_REUSEPORT, &enable2, sizeof(int));
    Bind(listenFdSearchPeer,(struct sockaddr*)&serverAddrSearch,sizeof(serverAddrSearch));
    Listen(listenFdSearchPeer,sizeQueue);    
    //------------------------------------------------//

    
    if(listenFdRegisterPeer>listenFdSearchPeer)
        maxFd=listenFdRegisterPeer;
    else
        maxFd=listenFdSearchPeer;
    

    FD_ZERO(&readSet);

    while(true)
    {

        FD_SET(listenFdRegisterPeer,&readSet);
        FD_SET(listenFdSearchPeer,&readSet);

        fflush(stdin);

        
        printf("\nServer in ascolto\n\n");
        if( select(maxFd+1,&readSet,NULL,NULL,NULL) < 0 )
        {
            perror("errore select\n");
            exit(-1);
        }


        if(FD_ISSET(listenFdRegisterPeer,&readSet))//CASO IN CUI VIENE NOTIFICATA UNA NUOVA CONNESSIONE
        {
            printf("Nuovo peer connesso\n");
            clientFd=Accept(listenFdRegisterPeer,(struct sockaddr *)&clientAddr,&sizeClientAddr);

            if (!fork())//FORKO SU UN PROCESSO FIGLIO
            {
                close(listenFdRegisterPeer);//CHIUDO L'FD DI ASCOLTO
                
                FullRead(clientFd, &peerInfo,sizeof(PeerInfo));//IL CLIENT FORNISCE IL SUO IDENTIFICATIVO, IP E PORTA SONO GIA STATE OTTENUTE TRAMITE ACCEPT
                inet_ntop(AF_INET, &clientAddr.sin_addr.s_addr, peerInfo.ipAddress,sizeof(peerInfo.ipAddress));
                //CONVERTO DA NETWORK ORDER A PRESENTATION PER OTTENERE L'IP COME STRINGA NORMALE

                //printf("\nProvo ad aggiungere");
                addInfo(&peerInfo);//AGGIUNGO AL FILE LE INFO DEL PEER
                close(clientFd);
                exit(0);
            }
            close(clientFd);
        }

        if(FD_ISSET(listenFdSearchPeer,&readSet))//CASO IN CUI VIENE RICHIESTA UNA INFO SU UN PEER
        {
            clientFd=Accept(listenFdSearchPeer,NULL,NULL);

            //printf("MI CHIEDONO INFO\n");


            if (!fork()){//FORKO - DALLA ACCEPT NON PRENDO IP E PORTA DATO CHE NON SERVONO IN QUESTO CASO
                close(listenFdSearchPeer);

                //LEGGO DAL CLIENT L'ID DEL PEER DI CUI VUOLE CONOSCERE LE INFORMAZIONI
                FullRead(clientFd, &peerInfo.idPeer,sizeof(int));

                //LO SI RICERCA ALL'INTERNO DEL FILE, E SE NON E' PRESENTE SI RITORNA UN PEERINFO CON ID -1
                searchPeer(&peerInfo);
                
                printf("Invio un peer info con IP=%s, chPort=%d, upPort=%d, ID=%d\n", peerInfo.ipAddress, peerInfo.channelPort, peerInfo.updatePort, peerInfo.idPeer);

                //LO INVIAMO E CHIUDIAMO LA CONNESSIONE
                FullWrite(clientFd,&peerInfo,sizeof(PeerInfo));
                close(clientFd);
                exit(0);
            }
            close(clientFd);
        }
    }
}

void signal_handler(int signal){
    
    printf("Processo stoppato, lo aborto\n");
    raise(SIGABRT);
    
}