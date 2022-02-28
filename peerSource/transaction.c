#include "header.h"

//FUNZIONE UTILIZZATA PER RICERCARE UN POSSIBILE STATE CHANNEL SUL QUALE INOLTRARE LA TRANSAZIONE
int searchAnExitWay( Transaction tempTransaction, TransactionNode *tempTransactionNode, PeerToReach *tempPeerToReach)
{
    int i, j;
    StateChannel *tempStateChannel;
    
    if(tempTransactionNode==NULL)
        tempTransactionNode=searchTransaction(tempTransaction.id,tempTransaction.idSenderPeer,temporaryTransactions);

    //verifico l'esistenza di statechannels non ancora considerati dove inoltrare la transazione
    for (i = 0; i < tempPeerToReach->sizeArrayExitWays; i++)
    {
        for (j = 0; j < MAX_STATE_CHANNEL; j++)
        {
            //se lo state channel corrente già è stato considerato allora ignoralo
            if (tempTransactionNode->alreadyExploredExitWays[j] == tempPeerToReach->arrayExitWays[i])
            { 
                break;
            }
        }
        //se ho scorso tutto il ciclo interno e quindi assodo che lo state channel non è stato ancora considerato e 
        //verifico che esso non sia lo stesso da dove proviene la transazione vi inoltro la stessa.
        if ((j == MAX_STATE_CHANNEL)&&(tempPeerToReach->arrayExitWays[i]!=tempTransactionNode->transaction.idPrevPeer))
        { 
            //È GIA STATA SETTATA IN CORSO, QUINDI MI ACCINGO AD INVIARLA ALL'I-ESIMO PEER
            pthread_mutex_lock(&mutexMyChannels);
            tempStateChannel = searchIntoList(myChannels, tempPeerToReach->arrayExitWays[i]);
            //CERCO LE INFO SULLO STATE CHANNEL A CUI INVIARE QUESTA TRANSAZIONE
            if (tempStateChannel->committedValue >= tempTransactionNode->transaction.value)
            {
                tempStateChannel->committedValue -= tempTransaction.value; //IMPEGNO LA VALUTA
                pthread_mutex_unlock(&mutexMyChannels);
                
                j = 0;
                while (tempTransactionNode->alreadyExploredExitWays[j] != 0)
                    j++;
                tempTransactionNode->alreadyExploredExitWays[j] = tempPeerToReach->arrayExitWays[i];
                //  /|\ AGGIUNGO IL PEER A QUELLI ESPLORATI
                sendATransaction(tempPeerToReach->arrayExitWays[i],tempTransaction);
                return 1;
            }
             pthread_mutex_unlock(&mutexMyChannels);
             
        }
    }
    return 0;
}

//------------------------------------------GESTORE DELLE TRANSAZIONI----------------------------------------//
void transactionHandler(void * args)
{
    StateChannel *prevStateChannel,*tempStateChannel;
    Transaction tempTransaction=*((Transaction*) args);
    TransactionNode *tempTransactionNode;
    FILE * transactionsCommittedFile;

    //SE SONO IL DESTINATARIO DELLA TRANSAZIONE
    if(tempTransaction.idReceiverPeer==myInfo.idPeer) 
    {
        pthread_mutex_lock(&mutexMyChannels);
        //ricerco il canale dove inoltrare la transazione
        prevStateChannel = searchIntoList(myChannels, tempTransaction.idPrevPeer);
        pthread_mutex_unlock(&mutexMyChannels);
        //Mi accredito sullo state channel da dove proviene la transazione il valore riportato dalla stessa.
        prevStateChannel->committedValue += tempTransaction.value;

        tempTransaction.state = TR_COMPIUTA;
        
        //----------------------------------------- DA FARE
        //SALVO LA TRANSAZIONE COME PERMANENTE
        printf("//---------------//\n//-Transazione da peer #%d con valore %d è stata ricevuta\n//---------------//\n",tempTransaction.idSenderPeer,tempTransaction.value);
        //invio la transazione compiuta allo state channel precedente.
        sendATransaction(tempTransaction.idPrevPeer, tempTransaction);
        //salvo la transazione ricevuta in maniera permanente all' interno del file "transactions.txt"
        transactionsCommittedFile=fopen("transactions.txt","a");
        fprintf(transactionsCommittedFile,"RICEVUTA -MITT peer %d -VAL %d\n",tempTransaction.idSenderPeer,tempTransaction.value);
        fclose(transactionsCommittedFile);
        printf("BILANCIO ATTUALE: %d\n", myBalance);
        printf("|--- IN ASCOLTO ---|\n1. Chiudi uno state channel.\n2. Effettua transazione\n3. Vedi gli state channel aperti.\n|----------------|\n");
        pthread_exit((void*)0);
    }

    //SE IL CODICE STATO E' "TRANSAZIONE COMPIUTA"
    if(tempTransaction.state==TR_COMPIUTA)
    {
        //SE SONO IL MITTENTE
        if (tempTransaction.idSenderPeer == myInfo.idPeer) 
        {
             printf("//---------------//\n//-Transazione #%d è stata eseguita\n//---------------//\n\n",tempTransaction.id);
             //salvo la transazione ricevuta in maniera permanente all' interno del file "transactions.txt"
            transactionsCommittedFile=fopen("transactions.txt","a");
            fprintf(transactionsCommittedFile,"EFFETTUATA -ID #%d -DEST peer %d -VAL %d\n",tempTransaction.id,tempTransaction.idReceiverPeer,tempTransaction.value);
            fclose(transactionsCommittedFile);
            printf("BILANCIO ATTUALE: %d\n", myBalance);
            printf("|--- IN ASCOLTO ---|\n1. Chiudi uno state channel.\n2. Effettua transazione\n3. Vedi gli state channel aperti.\n|----------------|\n");
        }
        else 
        {
            //ricerco la transazione temporanea al fine di ottenere l'id del peer che mi ha inviato inizialmente la transazione
            tempTransactionNode=searchTransaction(tempTransaction.id,tempTransaction.idSenderPeer,temporaryTransactions);

            pthread_mutex_lock(&mutexMyChannels);
            tempStateChannel=searchIntoList(myChannels, tempTransactionNode->transaction.idPrevPeer);
            //Mi accredito sullo state channel da dove proviene la transazione il valore riportato dalla stessa.
            tempStateChannel->committedValue += tempTransaction.value;
            pthread_mutex_unlock(&mutexMyChannels);
            
            //MANDO LA TRANSAZIONE AL MIO PRECEDENTE IL CUI ID È STATO SALVATO NELLE TRANSAZIONI TEMPORANEE
            sendATransaction(tempTransactionNode->transaction.idPrevPeer, tempTransaction);
        }
        //CANCELLO LA TRANSAZIONE DALLE TEMPORANEE
        deleteTransaction(tempTransaction.id, tempTransaction.idSenderPeer, temporaryTransactions); 
        pthread_exit((void *)0);
    }

    if(tempTransaction.state==TR_IN_CORSO) 
    {
        tempTransactionNode=searchTransaction(tempTransaction.id,tempTransaction.idSenderPeer,temporaryTransactions);

        if(tempTransactionNode!=NULL) //SE GIA' ESISTE UNA TRANSAZIONE TEMPORANEA CON LO STESSO ID///evito i loop///
        {
            tempTransaction.state=TR_ANNULLATA;
            
            //RIMANDO LA TRANSAZIONE ALL'ULTIMO CHE L'HA GESTITA PER EVITARE I LOOP INDICANDOGLI CHE QUESTA TRANSAZIONE È STATA ANNULLATA
            sendATransaction(tempTransaction.idPrevPeer, tempTransaction);
            pthread_exit((void *)0);
        }
        else
        {
            //se mi trovo a gestire per la prima volta questa transazione la salvo nell'hash table delle transazioni gestite.
            tempTransactionNode=(TransactionNode*)malloc(sizeof(TransactionNode));
            tempTransactionNode->transaction=tempTransaction;
            insertTransaction(tempTransactionNode,temporaryTransactions);
        }
    }

    //SE IL CODICE STATO E' "TRANSAZIONE INCOMPIUTA"
    if(tempTransaction.state==TR_ANNULLATA)
    {
        pthread_mutex_lock(&mutexMyChannels);
        prevStateChannel = searchIntoList(myChannels, tempTransaction.idPrevPeer);
        //SBLOCCO LE RISORSE DEL CANALE IMPEGNATE
        (prevStateChannel->committedValue)+= tempTransaction.value;
        pthread_mutex_unlock(&mutexMyChannels);
        //SETTO LO STATO SU IN_CORSO PER VALUTARE L'ESISTENZA DI ULTERIORI PERCORSI
        tempTransaction.state=TR_IN_CORSO;
    }


    PeerToReach* tempPeerToReach;

    //verifico l'esistenza di un PeerToReach in modo da poter valutare i possibile state channels dove inoltrare la transazione
    pthread_mutex_lock(&mutexPeerICanReach);
    tempPeerToReach=searchPeer(tempTransaction.idReceiverPeer, myPeerICanReach);
    pthread_mutex_unlock(&mutexPeerICanReach);

    if (tempPeerToReach!=NULL){
        if (searchAnExitWay(tempTransaction,tempTransactionNode,tempPeerToReach))
             pthread_exit(0);
    }



    //se sono il mittente e mi ritorna la transazione con stato TR_ANNULLATA creo uno state channel con il peer interessato
    if (tempTransaction.idSenderPeer == myInfo.idPeer) 
    { 
        printf("|-----> La transazione NON può essere effettuata.\n|------> Apri uno state channel col peer: \n");
        if (!openStateChannel())
            printf("Richiesta apertura canale rifiutata.\n\n");
        else
            printf("State channel creato\n\n");
        printf("BILANCIO ATTUALE: %d\n", myBalance);
        printf("|--- IN ASCOLTO ---|\n1. Chiudi uno state channel.\n2. Effettua transazione\n3. Vedi gli state channel aperti.\n|----------------|\n");
    }
    else
    { // SE NON SONO IL MITTENTE COMUNICO INDIETRO (A CHI MI HA INIZIALMENTE INVIATO LA TRANSAZIONE) CHE NON é POSSIBILE CONTINUARE DA ME
        tempTransactionNode=searchTransaction(tempTransaction.id, tempTransaction.idSenderPeer, temporaryTransactions);
        //CERCO LA TRANSAZIONE NELLE MIE TEMPORANEE
        tempTransaction=tempTransactionNode->transaction;
        //SETTO LO STATO DELLA TRANSAZIONE DA INVIARE INDIETRO
        tempTransaction.state=TR_ANNULLATA;
        //---//
        sendATransaction(tempTransaction.idPrevPeer, tempTransaction);
    }
    //Elimino la transazione dalle transazioni gestite
    deleteTransaction(tempTransaction.id,tempTransaction.idSenderPeer,temporaryTransactions);
    pthread_exit(0);
}
//---------------------------------------------------------------------//
void transactionListener (void*args)
{
    //SI METTE IN ASCOLTO
    int listenFdTransaction,enable=1,acceptedFdTransaction;
    pthread_t tid;

    Transaction tempTransaction;


    //Creo una socket che permette di restare in ascolto di transazioni
    listenFdTransaction=Socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    socklen_t sizeAddr=(socklen_t)sizeof(struct sockaddr_in);
    addr.sin_port=htons(myInfo.transactionPort);
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_ANY);

    setsockopt(listenFdTransaction, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &enable, sizeof(int));

    Bind(listenFdTransaction,(struct sockaddr*)&addr,sizeof(addr));
    Listen(listenFdTransaction,1000);

    while(1){

        //RICEVE UNA TRANSAZIONE
        acceptedFdTransaction=Accept(listenFdTransaction, (struct sockaddr*)&addr, &sizeAddr);
        FullRead(acceptedFdTransaction, &tempTransaction, sizeof(Transaction));
        close(acceptedFdTransaction);
        //LEGGO LA TRANSAZIONE
        pthread_create(&tid,NULL,(void*) transactionHandler, (void*)&tempTransaction);
        pthread_detach(tid);
    }
}

//-----------------------------------INVIO EFFETTIVO DELLE TRANSAZIONI------------------------------------//
void sendATransaction(int idPeer, Transaction transaction)
{
    int sockFd;
    StateChannel *stateChannel;
    struct sockaddr_in addr;

    pthread_mutex_lock(&mutexMyChannels);
    //reupero lo state channel che contiene le informazioni per la connessione al peer interessato
    stateChannel=searchIntoList(myChannels,idPeer);
    pthread_mutex_unlock(&mutexMyChannels);
    
     //Quando mando una transazione colui che la riceve può sapere l'ultimo peer che l'ha gestita mediante l'attributo idPrevPeer
    transaction.idPrevPeer=myInfo.idPeer;
    sockFd = Socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_port = htons(stateChannel->info.transactionPort);
    addr.sin_addr.s_addr = inet_addr(stateChannel->info.ipAddress);
    addr.sin_family = AF_INET;


    Connect(sockFd, (struct sockaddr *)&addr, sizeof(addr));
    FullWrite(sockFd, &transaction, sizeof(Transaction)); //RITORNO AL PRECEDENTE L'ESITO POSITIVO
    close(sockFd);
}
    
    
