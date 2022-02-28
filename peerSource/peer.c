#include "header.h"


void signal_handler(int signal); 


int main (int argc, char *argv[]){

    if(argc<6)
    {
        printf("Avvio incorretto.\n Eseguire con ./peer ID_PEER IP_SERVER CHANNEL_PORT UPDATE_PORT TRANSACTION_PORT\n");
        exit(0);
    }
    signal(SIGINT,signal_handler);
    signal(SIGTSTP,signal_handler);
    //GESTIONE DELLE CHIUSURE PROCESSI, LASCIAMO CHE I THREAD RILASCINO LE RISORSE PRIMA DI TERMINARE DEFINITVAMENTE

    pthread_t tid[3];
    myChannels=NULL;
    *myPeerICanReach = NULL;
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
        temporaryTransactions[i] = NULL;

    //POPOLO LA PEERINFO GLOBALE DELLE MIE INFO, CHE INVIERÒ OGNI VOLTA AI PEER CON IL QUALE VOGLIO CONNETTERMI
    myInfo.idPeer=atoi(argv[1]);
    myInfo.channelPort=atoi(argv[3]);
    myInfo.updatePort=atoi(argv[4]);
    myInfo.transactionPort=atoi(argv[5]);
    //------------------//
    

    FILE * myFile;
    myFile=fopen("initialize.txt","r+");
    fscanf(myFile,"bilancio:%d\nlastTransactionId:%d",&myBalance,&lastTransactionId);
    fclose(myFile);
    
    printf("Bilancio:%d \t LastId:%d ",myBalance,lastTransactionId);
    //COPIO L'IP DEL SERVER
    strcpy(serverIp, argv[2]);

    notifyConnection(myInfo.idPeer, myInfo.channelPort, myInfo.updatePort);//NOTIFICO LA MIA CONNESSIONE E I MI DATI AL SERVER
    printf("Connessione notificata\n");

    pthread_mutex_init(&mutexPeerICanReach,NULL);
    pthread_mutex_init(&mutexMyChannels,NULL);

  
    //THREAD CHE GESTISCE L'INTERFACCIAMENTO CON L'UTENTE
    //IN PARTICOLARE: APERTURA CANALI, RICHIESTE DA ALTRI DI APERTURE CANALI E CREAZIONE TRANSAZIONI.
    pthread_create(&tid[0],NULL, (void*)humanInteraction,NULL);
    pthread_create(&tid[1], NULL, (void*)updatePeersToReach, NULL);
    pthread_create(&tid[2], NULL, (void*)sendPeersToReach, NULL);
    pthread_create(&tid[3], NULL, (void*)transactionListener, NULL);

    pthread_join(tid[0],NULL);
    pthread_join(tid[1],NULL);
    pthread_join(tid[2],NULL);
    pthread_join(tid[3],NULL);
}

void signal_handler(int signal){
    
    printf("Processo stoppato, lo aborto\n");
    FILE * myFile;
    myFile=fopen("initialize.txt","w");
    
    while(myChannels!=NULL){
        printf("Chiudo il canale CON PEER %d\n", myChannels->info.idPeer);
        closeStateChannel(myChannels->info.idPeer);
    }
    
    fprintf(myFile,"bilancio:%d\nlastTransactionId:%d",myBalance,lastTransactionId);
    fclose(myFile);

    pthread_mutex_destroy(&mutexPeerICanReach);
    pthread_mutex_destroy(&mutexMyChannels);
    
    raise(SIGABRT);
}


