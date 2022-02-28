#include "header.h"

//FUNZIONE DI HASH
int hashCodeTr(int idTransKey, int idSenderPeerKey){
    return ((idTransKey*17)+(idSenderPeerKey)*37)%HASH_TABLE_SIZE;
}

TransactionNode* searchTransaction(int idTransKey, int idSenderPeerKey, TransactionNode* hashArray[]){

    int indice=hashCodeTr(idTransKey,idSenderPeerKey);
    TransactionNode* temp;
    temp=hashArray[indice];
    while(temp != NULL)
    {
        if(temp->transaction.id==idTransKey && temp->transaction.idSenderPeer==idSenderPeerKey) 
            return temp; //SE LA CHIAVE CORRISPONDE HO TROVATO L'ELEMENTO
        else 
            temp=temp->nextTransaction; //ALTRIMENTI PROVO L'ELEMENTO SUCCESSIVO
    }
    
    return NULL; //SE USCIAMO DAL WHILE, ALLORA NON ABBIAMO TROVATO  NULLA
}

//-----------------------------------------------------------//

void insertTransaction(TransactionNode* transactionToInsert, TransactionNode* hashArray[]){

    if((searchTransaction(transactionToInsert->transaction.id, transactionToInsert->transaction.idSenderPeer,hashArray)) == NULL){//SE L'ELEMENTO NON E` GIA STATO INSERITO
    
        printf("Elemento costruito\n");
        //INSERIMENTO IN TESTA
        int indice=hashCodeTr(transactionToInsert->transaction.id,transactionToInsert->transaction.idSenderPeer);
        if(hashArray[indice]==NULL)
            hashArray[indice]=transactionToInsert;
        else
        {
           transactionToInsert->nextTransaction=hashArray[indice];
           hashArray[indice]=transactionToInsert;
        }
    }

}

//-----------------------------------------------------------//


void deleteTransaction(int idTransKey, int idSenderPeerKey, TransactionNode* hashArray[]){

    //PRENDO LA CHIAVE DELL'ELEMENTO

    int indice=hashCodeTr(idTransKey,idSenderPeerKey);

    TransactionNode **elementOfList, **oldElementOfList=NULL;
    elementOfList=&hashArray[indice];


    while(*elementOfList!=NULL){
        
        if(((*elementOfList)->transaction.id !=idTransKey) && ((*elementOfList)->transaction.idSenderPeer!=idSenderPeerKey)){
            oldElementOfList=elementOfList;//TENGO TRACCIA DEL VECCHIO ELEMENTO
            elementOfList=&((*elementOfList)->nextTransaction);
        }
        else{//ALTRIMENTI SE HO TROVATO L'OGGETTO DA ELIMINARE

            if(oldElementOfList==NULL){//---------------------------------------SE STO IN CIMA ALLA LISTA
                hashArray[indice]=(*elementOfList)->nextTransaction;
                free(*elementOfList);
            }
            else if(((*elementOfList)->nextTransaction) == NULL){//-------------------------SE SONO L'ULTIMO DELLA LISTA
                (*oldElementOfList)->nextTransaction=NULL;
                free(*elementOfList);
            }
            else{//-------------------------------------------------------------SE SONO UN ELEMENTO DI MEZZO
                oldElementOfList=&(*elementOfList)->nextTransaction;
                free(*elementOfList);
            }
        }

    }

}
