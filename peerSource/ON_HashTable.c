#include "header.h"

//FUNZIONE DI HASH
int hashCodePTR(int idPeerToReach){
    return idPeerToReach%HASH_TABLE_SIZE;
}

PeerToReach* searchPeer(int idPeerToReach, PeerToReach* hashArray[]){

    //USO LA FUNZIONE DI HASH
    int indice=hashCodePTR(idPeerToReach);

    PeerToReach* elementFound;
        
    elementFound=hashArray[indice];

    while(elementFound != NULL){
            
        if(elementFound->idPeerToReach == idPeerToReach) return elementFound; //SE LA CHIAVE CORRISPONDE HO TROVATO L'ELEMENTO
            //------//
        else elementFound=elementFound->nextPeerToReach; //ALTRIMENTI PROVO L'ELEMENTO SUCCESSIVO

    }
    
    return NULL; //SE USCIAMO DAL WHILE, ALLORA NON ABBIAMO TROVATO  NULLA
}

//-----------------------------------------------------------//

PeerToReach* insertPeer(int idPeerToReach, PeerToReach* hashArray[]){

    PeerToReach *PeerToReachToInsert;

    if((PeerToReachToInsert=searchPeer(idPeerToReach, hashArray)) == NULL){//SE L'ELEMENTO NON E` GIA STATO INSERITO
        
        PeerToReach *PeerToReachToInsert=(PeerToReach*) malloc(sizeof(PeerToReach));
        PeerToReachToInsert->idPeerToReach=idPeerToReach;
        PeerToReachToInsert->arrayExitWays=NULL;
        PeerToReachToInsert->sizeArrayExitWays=0;
        PeerToReachToInsert->nextPeerToReach=NULL;
        
        //COSTRUISCO IL MIO PeerToReach DA INSERIRE, TROVO MEDIANTE FUNZIONE DI HASH L'INDICE DELL'ARRAY IN CUI INSERIRE L'ELEMENTO PEERTOREACH
        int indice=hashCodePTR(idPeerToReach);

        PeerToReach** elementOfList;    //Puntatore a *elementOfList

        //Mi posiziono esattamente sull'elemento dell'hashArray con quell'indice
        elementOfList=&hashArray[indice]; //ELEMENTOFLIST E` UGUALE ALl'INDIRIZZO DEL PUNTATORE DEL PRIMO ELEMENTO DELLA LISTA CORRISPONDENTI A QUELL'INDICE (PUO ANCHE ESSERE NULL)


        while(*elementOfList !=NULL) { printf("elemento non null\n"); elementOfList=&((*elementOfList)->nextPeerToReach);} //SCORRO LA LISTA ASSOCIATA ALL'I-ESIMO ELEMENTO DI HASHARRAY

        //QUANDO SARA' NULL, POTREMO SOSTITUIRLO CON IL NOSTRO ELEMENTO DA INSERIRE
        *elementOfList=PeerToReachToInsert;
        printf("ELEMENTO COSTRUITO\n");
        return PeerToReachToInsert;
    }
    else 
        return PeerToReachToInsert;
}

//-----------------------------------------------------------//

int deletePeer(int idPeerToReach, PeerToReach* hashArray[]){

    int indice=hashCodePTR(idPeerToReach);

    //Se non dichiarassi il doppio puntatore a cui do l'indirizzo di hashArray[indice] quest'ultimo subirebbe delle modifiche non permanenti al di fuori della funzione
    PeerToReach **elementOfList, **oldElementOfList=NULL;
    elementOfList=&hashArray[indice];


    while(*elementOfList!=NULL){
        
        if((*elementOfList)->idPeerToReach !=idPeerToReach){
            oldElementOfList=elementOfList;//TENGO TRACCIA DEL VECCHIO ELEMENTO
            elementOfList=&((*elementOfList)->nextPeerToReach);
        }
        else{//ALTRIMENTI SE HO TROVATO L'OGGETTO DA ELIMINARE

            if(oldElementOfList==NULL){//---------------------------------------SE STO IN CIMA ALLA LISTA
                hashArray[indice]=(*elementOfList)->nextPeerToReach;
                free(*elementOfList);
            }
            // ??????? C'È UN ERRORE: è * non &((*elementOfList))
            else if(((*elementOfList)->nextPeerToReach) == NULL){//-------------------------SE SONO L'ULTIMO DELLA LISTA
                (*oldElementOfList)->nextPeerToReach=NULL;
                free(*elementOfList);
            }
            else{//-------------------------------------------------------------SE SONO UN ELEMENTO DI MEZZO
                (*oldElementOfList)->nextPeerToReach=(*elementOfList)->nextPeerToReach;
                free(*elementOfList);
            }
        }
        return 1;
    }

    return 0;
}

