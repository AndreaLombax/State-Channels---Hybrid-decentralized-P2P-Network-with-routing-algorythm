#include "header.h"

int openStateChannel()
{
    ConnectionRequest connectionRequest;
    int clientFd;
    int idReceiverPeer;
    struct sockaddr_in addr;

    printf("Inserisci identificativo del peer: ");
    idReceiverPeer=insertInput();

    
    do
    {
        printf("Valore da impegnare nel canale: ");
        connectionRequest.usedValue=insertInput();
    }while(connectionRequest.usedValue>myBalance);

    //ritorna false se si vuole aprire uno stateChannel già aperto o si vuole aprire uno state channel con se stessi
    if((searchIntoList(myChannels, idReceiverPeer)!=NULL) || (idReceiverPeer==myInfo.idPeer)){
        printf("ERRORE: Canale gia' aperto.\n");
        pthread_mutex_unlock(&mutexMyChannels);
        return false;
    }
    pthread_mutex_unlock(&mutexMyChannels);

    //------------------------//COMUNICAZIONE COL SERVER
    PeerInfo peerInfoDest;

    peerInfoDest = getPeerInfo(idReceiverPeer);
    //PRENDO LE INFO DEL PEER A CUI VOGLIO CONNETTERMI CHIEDENDOLE AL SERVER

    connectionRequest.info=myInfo;//DO LE MIE INFO AL PEER CON CUI EVENTUALMENTE MI CONNETTO
    if (peerInfoDest.idPeer != -1){
        //SETTO LA SOCKET PER CONNETTERMI
        clientFd = Socket(AF_INET, SOCK_STREAM, 0);
        addr.sin_port = htons(peerInfoDest.channelPort);
        addr.sin_addr.s_addr = inet_addr(peerInfoDest.ipAddress);
        addr.sin_family = AF_INET;
        sleep(1);
        if(Connect(clientFd, (struct sockaddr *)&addr, sizeof(addr))==-1)
        {
            return 0;
        }
        //FINE SETTING E CONNESSIONE

        //Salvo la valuta impegnata dal peer1 prima di perderla in quanto connectionRequest sarà sovrascritto da una read
        int valutaImpegnata = connectionRequest.usedValue;
        connectionRequest.confirmed=1;

        //invio al peer la richiesta
        if(FullWrite(clientFd, &connectionRequest, sizeof(ConnectionRequest))==0){
            close(clientFd);
            return false;
        }

        //ricevo la richiesta dal peer
        if(FullRead(clientFd, &connectionRequest, sizeof(ConnectionRequest))==0){
            close(clientFd);
            return false;
        }

        close(clientFd);
        if (connectionRequest.confirmed){//SE MI CONFERMA L'APERTURA DELLO STATE CHANNEL LO AGGIUNGO AI MIEI SC DIRETTI

            myBalance-=valutaImpegnata;

            //aggiungi lo state channel nella lista
            StateChannel *StateChannelToInsert=(StateChannel*)malloc(sizeof(StateChannel));
            StateChannelToInsert->channelState=connectionRequest.usedValue+valutaImpegnata;//Valuta impegnata peer2+ Valuta impegnata peer1
            StateChannelToInsert->committedValue=valutaImpegnata;//Valore impegnato dal peer principale(colui che ha avviato il processo) in uno state channel
            StateChannelToInsert->info=peerInfoDest;
            StateChannelToInsert->nextStateChannel=NULL;
            pthread_mutex_lock(&mutexMyChannels);
            insertIntoList(&myChannels, StateChannelToInsert);
            pthread_mutex_unlock(&mutexMyChannels);
            
            return true;
        }
    }
    return false;
}

void openChannelRequest(int clientFd, char clientIp[]){

    int myCommittedValue;
    ConnectionRequest connectionRequestToMe;
    sleep(1);
    //ACCETTO IL PACCHETTO DI APERTURA CANALE E VALUTO L'APERTURA
    FullRead(clientFd, &connectionRequestToMe, sizeof(ConnectionRequest));

    printf("RICHIESTA DA PEER %d:\n",connectionRequestToMe.info.idPeer);
    //SE LA RICHIESTA E` UNA RICHIESTA DI CHIUSURA...
    if(connectionRequestToMe.confirmed==-1){
        closeStateChannelRequest(connectionRequestToMe);
        close(clientFd);
        return; 
    }
    //---------------------------------------------//ALTRIMENTI

    printf("|---| Accetta richiesta -> 1\n|---| Rifiuta richiesta -> 0\n\n ");
    
    do{
        printf("|---| Inserisci scelta: ");
        connectionRequestToMe.confirmed=insertInput();
    }while(connectionRequestToMe.confirmed != 1 && connectionRequestToMe.confirmed != 0);

    if(connectionRequestToMe.confirmed==1){//SIGNIFICA CHE HO ACCETTATO LA CREAZIONE DELLO STATE CHANNEL
        
    
        do
        {
            printf("|---| Inserisci la valuta da impiegare nel canale: ");
            myCommittedValue=insertInput();
        }while (myCommittedValue > myBalance);
        
        myBalance-=myCommittedValue;

        //aggiungi lo state channel in hash table
        StateChannel *StateChannelToInsert=(StateChannel*)malloc(sizeof(StateChannel));

        StateChannelToInsert->channelState=connectionRequestToMe.usedValue+myCommittedValue;
        StateChannelToInsert->committedValue=myCommittedValue;
        StateChannelToInsert->info=connectionRequestToMe.info;
        strcpy(StateChannelToInsert->info.ipAddress, clientIp);
        StateChannelToInsert->nextStateChannel=NULL;

        pthread_mutex_lock(&mutexMyChannels);
        insertIntoList(&myChannels, StateChannelToInsert);
        pthread_mutex_unlock(&mutexMyChannels);
    }
    
    //INVIAMO LA RISPOSTA
    FullWrite(clientFd, &connectionRequestToMe, sizeof(ConnectionRequest));
    close(clientFd);
    
}