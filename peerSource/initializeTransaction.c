#include "header.h"

void initializeTransaction()
{
    printf("///START initializeTransaction\n");
    Transaction transactionRequest;
    PeerToReach* tempPeerToReach;
    TransactionNode* tempTransactionNode=(TransactionNode*)calloc(1,sizeof(TransactionNode));
    printf("Inserisci il destinatario di questa transazione:");
    transactionRequest.idReceiverPeer=insertInput();
    
    printf("Inserisci il valore di questa transazione:");
    transactionRequest.value=insertInput();

    while(transactionRequest.value>myBalance)
    {
         printf("Valore non valido, Inserire nuovo valore:");
        transactionRequest.value=insertInput();
    }

    transactionRequest.id = lastTransactionId++;
    transactionRequest.idPrevPeer = myInfo.idPeer;
    transactionRequest.idSenderPeer = myInfo.idPeer;
    transactionRequest.state = TR_IN_CORSO;
    tempTransactionNode->transaction=transactionRequest;
    insertTransaction(tempTransactionNode,temporaryTransactions);
    tempPeerToReach=searchPeer(transactionRequest.idReceiverPeer, myPeerICanReach);
    //CASO IN CUI POSSO RAGGIUNGERE UN PEER SFRUTTANDO LA CONNESSIONE CON UNO DEI MIEI PEER DIRETTI
    if (tempPeerToReach!=NULL)
        if (searchAnExitWay(transactionRequest,tempTransactionNode,tempPeerToReach))
            return;

    printf("|-----> La transazione NON può essere effettuata.\n|------> Apri uno state channel col peer: \n");
    if (!openStateChannel())
        printf("\nRichiesta apertura canale rifiutata.");
    else
        printf("\nState channel creato");

    deleteTransaction(transactionRequest.id,transactionRequest.idSenderPeer,temporaryTransactions);
}