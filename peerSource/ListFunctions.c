#include "header.h"

void insertIntoList(StateChannel**head, StateChannel*newStateChannel)
{
    newStateChannel->nextStateChannel=*head;
    *head=newStateChannel;
}

void deleteFromList(StateChannel**head,int stateChannelId)
{
    StateChannel** curr=head, **prev=(StateChannel**)malloc(sizeof(StateChannel*));
    *prev=NULL;  
    while(*curr!=NULL)
    {
        if((*curr)->info.idPeer==stateChannelId)
        {
            if(*prev==NULL){//CASO IN CUI STO ELIMINANDO LA TESTA
                *head=(*curr)->nextStateChannel;
                break;
            }
            else if((*curr)->nextStateChannel==NULL){//CASO IN CUI STO ALLA FINE
                (*prev)->nextStateChannel=NULL;
                break;
            }
            else{//CASO IN CUI STO NEL MEZZO
            printf("STO NEL MEZZO\n");
            printf("prev=%d, curr=%d, next del curr=%d\n", (*prev)->info.idPeer, (*curr)->info.idPeer, (*curr)->nextStateChannel->info.idPeer);
                (*prev)->nextStateChannel=(*curr)->nextStateChannel;
                //free(*curr);
                break;
            }
        }
        prev=curr;
        curr=&((*curr)->nextStateChannel);
    }
}

StateChannel*searchIntoList(StateChannel*head,int stateChannelId)
{
    while(head!=NULL){
        if(head->info.idPeer==stateChannelId)
            return head;
        head=head->nextStateChannel;
    }
    return head;
}

void printMyStateChannles()
{
    int i=1;
    StateChannel *temp=myChannels;
    printf("State channel attualmente aperti:\n");
    while(temp!=NULL){
        printf("%d) SC Aperto con il peer %d - Valore impiegato: %d - Stato del canale: %d\n\n",i++, temp->info.idPeer, temp->committedValue, temp->channelState);
        temp=temp->nextStateChannel;
    }
}
