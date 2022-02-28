#include "header.h"

// response= array dei peer da cui uscire per raggiungere idStateChannel che sarà l'exit ways per ogni response[i]
//Infatti ogni response[i] può essere raggiunto passando per idStateChannel
void updateHashTablePeersToReach(int *response, int receivedSize, int idStateChannel)
{
   
    PeerToReach *peerToUpdate;
    int i, j;
    bool alreadyExists;

    //Aggiorno tutti i peersToReach indicati da response con nuove exitways da usare per raggiungere gli stessi
    for (i = 0; i < receivedSize; i++)
    {
        if (response[i]==myInfo.idPeer)
            continue;
        alreadyExists = false;
        peerToUpdate = insertPeer(response[i], myPeerICanReach); //CERCO NELLA HASHTABLE ED EVENTUALMENTE INSERISCO
        peerToUpdate->sizeArrayExitWays;
        //Verifico che la exitWay non sia già stata assegnata al peerToReach identificato da response[i]
        for (j = 0; j < peerToUpdate->sizeArrayExitWays; j++)
        {
            if (peerToUpdate->arrayExitWays[j] == idStateChannel)
            {
                alreadyExists = true;
                break;
            }
        }
        //se la exitWay non è stata ancora assegnata
        if (!alreadyExists)
        {
            peerToUpdate->arrayExitWays = realloc(peerToUpdate->arrayExitWays, (peerToUpdate->sizeArrayExitWays + 1) * sizeof(int));
            peerToUpdate->arrayExitWays[peerToUpdate->sizeArrayExitWays] = idStateChannel;
            peerToUpdate->sizeArrayExitWays += 1;
        }
    }
}

//Funzione che permette di connettersi all'i-esimo peer che fornirà i propri PeersToReach (mediante updatePort)
void receivePeersToReach(void *args)
{
    StateChannel *curr = (StateChannel *)args;
    int requestFdUpdatePeers, receivedSize = 0,response[100000], receiverPort, enable=1;
    struct sockaddr_in destinationAddr;
    requestFdUpdatePeers = Socket(AF_INET, SOCK_STREAM, 0);
    //setsockopt(requestFdUpdatePeers, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &enable, sizeof(int));
    destinationAddr.sin_family = AF_INET;
    destinationAddr.sin_port = htons(curr->info.updatePort); //CHIEDO SULLA PORTA DEGLI UPDATE
    destinationAddr.sin_addr.s_addr = inet_addr(curr->info.ipAddress);
    

    if(Connect(requestFdUpdatePeers, (struct sockaddr *)&destinationAddr, sizeof(struct sockaddr_in))==-1){
        printf("Aggiornamento non disponibile dal peer %d\n", curr->info.idPeer);
        close(requestFdUpdatePeers);
        if(deletePeer(curr->info.idPeer, myPeerICanReach)==1){
            printf("Eliminato elemento dai peertoreach\n");
        }
        return;
    }

    //ricevo il size dell'array di interi che mi verrà inviato
    FullRead(requestFdUpdatePeers, &receivedSize, sizeof(receivedSize));
    //ricevo l'array di peer raggiungibili
    FullRead(requestFdUpdatePeers, response, sizeof(int) * receivedSize);
    //Attivo la function per aggiornare i peersToReach
    pthread_mutex_lock(&mutexPeerICanReach);
    updateHashTablePeersToReach(response, receivedSize, curr->info.idPeer);
    pthread_mutex_unlock(&mutexPeerICanReach);
    close(requestFdUpdatePeers);
}

//Funzione che crea un numero di thread pari ai miei state channel diretti al fine di aggiornare i peersToReach
void updatePeersToReach(void *args)
{

    int sizeTid=0;
    pthread_t *tid=NULL;
    StateChannel *curr = myChannels;

    //Per tutti i miei state channel diretti avvio un threadUpdatePeersToReach 

    
    while (1)
    {
        
        //Scorro tutti i miei stateChannel 
        while (curr != NULL)
        {
            tid = realloc(tid, (sizeTid + 1) * sizeof(pthread_t));
            //gestisco la connessione tramite thread
            pthread_create(tid + sizeTid, NULL, (void *)receivePeersToReach, (void *)curr);
            sizeTid++;
            curr = curr->nextStateChannel;
        }
        pthread_mutex_unlock(&mutexMyChannels);
        for (int i = 0; i < sizeTid; i++)
            pthread_join(tid[i], NULL);
        sleep(2);
        curr = myChannels;

        if(tid!=NULL)
            free(tid);
            
        tid=NULL;
        sizeTid=0;
    }

}



//FUNZIONE CHE SCORRE L'INTERA HASHTABLE DEI PEER CHE SI POSSONO RAGGIUNGERE
//E SE IL CAMPO DELLA HASH NON È VUOTO, AGGIUNGE IL PEER ALL'ARRAY CHE RITORNERÀ
void getPeersICanReach(int *arrayToSend, int *sizeArray)
{
    PeerToReach *tempPeer = (PeerToReach *)malloc(sizeof(PeerToReach));

    (*sizeArray)=0;

    for(int i = 0; i < HASH_TABLE_SIZE; i++){

        if (myPeerICanReach[i] != NULL){//CONTROLLO SE L'I-ESIMO ELEMENTO DELLA HASH è DIVERSO DA NULL
            (*sizeArray)++; //INCREMENTO IL SIZE PERCHE AGGIUNGERO UN NUOVO ELEMENTO
            
            arrayToSend[*sizeArray - 1] = myPeerICanReach[i]->idPeerToReach;

            //USO TEMPPEER PER SCORRERE L'EVENTUALE LISTA ASSOCIATA ALL'I-ESIMO ELEMENTO DELLA HASH
            tempPeer = myPeerICanReach[i];
            while (tempPeer->nextPeerToReach != NULL)
            {
                (*sizeArray)++;
                tempPeer = tempPeer->nextPeerToReach; //SE NON È NULL, VADO AL PROSSIMO
                arrayToSend[(*sizeArray) - 1] = myPeerICanReach[i]->idPeerToReach;
            }
            //SE ESCO HO FINITO DI SCORRERE L'I-ESIMA LISTA
        }

    } //FINE FOR

    //Includo me stesso nell'array di nodi raggiungibili
    arrayToSend[(*sizeArray)]=myInfo.idPeer;
    (*sizeArray)++;

}

//Funzione che serve ad inviare i PeersToReach a chi li richiede
void sendPeersToReach(void *args)
{
    int arrayToSendStatic[100000],sizeArray;
    int listenFdUpdatePackets, clientFd,enable = 1,*peersICanReach;
    struct sockaddr_in serverAddr;

    //SETTO LA SOCKET PER L'ASCOLTO DELLE RICHIESTE DI UPDATE
    listenFdUpdatePackets = Socket(AF_INET, SOCK_STREAM, 0);
    serverAddr.sin_port = htons(myInfo.updatePort);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //---------------------//

    setsockopt(listenFdUpdatePackets, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &enable, sizeof(int));
    Bind(listenFdUpdatePackets, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    Listen(listenFdUpdatePackets,20);
    while (true){
        clientFd=Accept(listenFdUpdatePackets,NULL,NULL);
        //OTTENGO I PEER CHE POSSO RAGGIUNGERE, E IN SEGUITO INVIO SIA IL SIZE CHE L'ARRAY STESSO
        pthread_mutex_lock(&mutexPeerICanReach);
        getPeersICanReach(arrayToSendStatic, &sizeArray);
        pthread_mutex_unlock(&mutexPeerICanReach);
        //Invio il size
        FullWrite(clientFd, &sizeArray, sizeof(sizeArray));
        //Invio l'array
        FullWrite(clientFd, arrayToSendStatic, sizeArray * sizeof(int));
        close(clientFd);
    }
    //------------------------------------------------//
}
