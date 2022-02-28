#include "header.h"

void closeStateChannel(int idStateChannelToClose){
    
    ConnectionRequest connectionRequest;
    
    connectionRequest.confirmed = -1;
    connectionRequest.info=myInfo;
    
    pthread_mutex_lock(&mutexMyChannels);
    StateChannel* stateChannelToDelete = searchIntoList(myChannels, idStateChannelToClose);
    pthread_mutex_unlock(&mutexMyChannels);

    //SE PROVO A CHIUSURA DI UN CANALE NON APERTO O DI UN CANALE CON ME
    if( (stateChannelToDelete==NULL) || (idStateChannelToClose==myInfo.idPeer)){
        printf("ERRORE: Chiusura di un canale non aperto.\n");
        return;
    }

    int stateChannelToCloseFd;
    struct sockaddr_in addr;

    //----------//
    stateChannelToCloseFd = Socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_port = htons(stateChannelToDelete->info.channelPort);
    addr.sin_addr.s_addr = inet_addr(stateChannelToDelete->info.ipAddress);
    addr.sin_family = AF_INET;

    //CREO LA SOCKET PER COMUNICARE AL CANALE LA CHIUSURA
    Connect(stateChannelToCloseFd, (struct sockaddr *)&addr, sizeof(addr));

    //SCRIVO AL CANALE UN REQUEST CON CONFIRMED=-1 PER INDICARGLI LA CHIUSURA
    FullWrite(stateChannelToCloseFd, &connectionRequest, sizeof(ConnectionRequest));

    myBalance+=stateChannelToDelete->committedValue;

    pthread_mutex_lock(&mutexMyChannels);
    deleteFromList(&myChannels, idStateChannelToClose);
    pthread_mutex_unlock(&mutexMyChannels);

    printf("PeerToReachToDelete:%d\n",searchPeer( idStateChannelToClose,myPeerICanReach)->idPeerToReach);
    if(deletePeer(idStateChannelToClose, myPeerICanReach)==1){
        printf("Eliminato anche dai PeerToReach\n");
    }
    close(stateChannelToCloseFd);
}

void closeStateChannelRequest(ConnectionRequest connectionRequest){

    printf("È stato chiuso il canale con il peer %d.\n", connectionRequest.info.idPeer);
    pthread_mutex_lock(&mutexMyChannels);
    StateChannel* stateChannelToDelete = searchIntoList(myChannels, connectionRequest.info.idPeer);
    myBalance+=stateChannelToDelete->committedValue;
    deleteFromList(&myChannels, connectionRequest.info.idPeer);
    if(deletePeer(connectionRequest.info.idPeer, myPeerICanReach)==1){
        printf("Eliminato anche dai PeerToReach\n");
    }
    
    pthread_mutex_unlock(&mutexMyChannels);

}