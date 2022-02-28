#include "headerServer.h"

//FUNZIONE DI RICERCA-----------------------------------//
void searchPeer(PeerInfo* peerInfo){

    FILE * filePtr;
    char *buffer, *token, delimiters[2]=" ";
    size_t sizeBuffer=sizeof(buffer);
    
    buffer=(char*)malloc(31);
    filePtr=fopen("info.txt","r");


    fseek(filePtr,0,SEEK_SET);//MI METTO ALL'INIZIO DEL FILE
    while(getline(&buffer,&sizeBuffer,filePtr)!=EOF)//SCORRO TUTTO IL FILE FINO A CHE NON TROVO L'HOST
    {
        if(atoi(buffer)==peerInfo->idPeer){//SE L'ID CORRISPONDE A QUELLO DA TROVARE
            printf("TROVATO\n");
            token = strtok(buffer, delimiters);

            for (int i=0; token!=NULL; i++ ){//COMPONGO LA MIA STRUCT
                
                //PRENDO ID
                if(i==0)
                    peerInfo->idPeer=atoi(token);
                //PRENDO IP
                else if(i==1)
                    strcpy(peerInfo->ipAddress,token);
                //PRENDO PORTA
                else if(i==2){
                    peerInfo->channelPort=atoi(token);
                    printf("channel port= %d\n", peerInfo->channelPort);
                }
                else if(i==3){
                    peerInfo->updatePort=atoi(token);
                    printf("update port= %d\n", peerInfo->updatePort);
                }
                else if(i==4){
                    peerInfo->transactionPort=atoi(token);
                    printf("transaction port= %d\n", peerInfo->transactionPort);
                }
                    
                //vado al prossimo token 
                token = strtok(NULL, delimiters);
            }
            fclose(filePtr);
            free(buffer);
            return;
        }
    }

    peerInfo->idPeer=-1;
    fclose(filePtr);
    free(buffer);
    return; // SE NON HO TROVATO NULLA RITORNO UN PEERINFO CON ID-1
}

//FUNZIONE DI AGGIORNAMENTO-----------------------------------//
void addInfo(PeerInfo* peerInfo){

    FILE *filePtr;
    char *buffer, *cleanBuffer;
    buffer=(char*)malloc(50);
    size_t sizeBuffer=sizeof(buffer);
    
    cleanBuffer=(char*)calloc(70,1);
    
    sprintf(cleanBuffer,"%d %s %d %d %d",peerInfo->idPeer,peerInfo->ipAddress,peerInfo->channelPort, peerInfo->updatePort, peerInfo->transactionPort);

    int size=strlen(cleanBuffer);
    for(int i=0; i<70-size;i++)
        strcat(cleanBuffer, " ");
    strcat(cleanBuffer, "\n");
    
    filePtr=fopen("info.txt","r+");

    fseek(filePtr,0,SEEK_SET);//MI METTO ALL'INIZIO DEL FILE
    
    while(getline(&buffer, &sizeBuffer, filePtr)!=EOF){

        if(strcmp(buffer,cleanBuffer)==0){//SE LE DUE LINNE SONO UGUALI NON FACCIO NULLA
            printf("Le due linee sono uguali\n");
            fclose(filePtr);
            free(buffer);
            free(cleanBuffer);
            return;
        }
        else if(atoi(buffer)==peerInfo->idPeer){//SE HO GIA REGISTRATO IL PEER MA HA CAMBIATO HOST O PORTA
            int pos=ftell(filePtr);
            fseek(filePtr, pos-strlen(buffer), SEEK_SET);
            printf("Sto per scrivere questo: %s\n", cleanBuffer);

            fprintf(filePtr, cleanBuffer);

            fclose(filePtr);
            free(buffer);
            free(cleanBuffer);
            return;
        }

    }
    fseek(filePtr, 0, SEEK_END);
    fprintf(filePtr, cleanBuffer);
    fclose(filePtr);
    free(buffer);
    free(cleanBuffer);
}