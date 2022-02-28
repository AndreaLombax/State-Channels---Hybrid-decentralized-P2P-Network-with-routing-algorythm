#ifndef HEADER_H_INCLUDED
#define HEADER_H_INCLUDED
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <pthread.h>
#include <malloc.h>
#include <stdbool.h>
#include <signal.h>

#define SERVER_NOTIFY_PORT 1500
#define SERVER_REQUEST_PORT 2520
#define HASH_TABLE_SIZE 10000
#define MAX_STATE_CHANNEL 10


typedef enum transactionState {TR_ANNULLATA = 0, TR_IN_CORSO = 1, TR_COMPIUTA = 2} transactionState; 

//Struct che contiene informazioni per usufruire dei servizi di un determinato peer
typedef struct PeerInfo
{
    int idPeer;
    char ipAddress[16];
    int channelPort;//PORTA CHE OFFRE IL SERVIZIO DI CREAZIONE CANALE
    int updatePort;//PORTA CHE OFFRE IL SERVIZIO DI AGGIORNAMENTI DI TABELLE DI ROUTING
    int transactionPort;//PORTA CHE OFFRE IL SERVIZIO DI GESTIONE DELLE TRANSAZIONI
}PeerInfo;


//ELEMENTO HASHTABLE
typedef struct StateChannel{
    int channelState;//STATO DEL CANALE
    int committedValue;//VALUTA IMPEGNATA NELL'APERTURA DEL CANALE
    PeerInfo info; //Informazioni per la connessione al peer
    struct StateChannel* nextStateChannel;//SE VI SONO COLLISIONI INSERISCO UN NUOVO PEER TO REACH NELLA LISTA MANTENUTA DA PEER_TO_REACH[i];
}StateChannel;

typedef struct PeerToReach{
    int idPeerToReach;//ID DEL PEER DA RAGGIUNGERE, FUNZIONERA' DA CHIAVE PER LA HASH TABLE
    int* arrayExitWays;//ARRAY DINAMICO DEI PEER DA CUI USCIRE PER RAGGIUNGERE idPeerToReach
    int sizeArrayExitWays;
    struct PeerToReach* nextPeerToReach; //PUNTATORE A NEXT CHE PERMETTE DI CREARE UNA LISTA CONCATENATA IN CASO DI COLLISIONI
}PeerToReach;
//------------------------------------------------//

typedef struct Transaction {
    int id;//Id della transazione
    int value;//Valore della transazione da spedire
    int idPrevPeer;//Id del peer precedente che ha gestito questa transazione
    int idReceiverPeer;//Id del peer che deve ricevere la transazione
    int idSenderPeer;//Id del peer che ha generato la transazione
    transactionState state;//Stato attuale della transazione
}Transaction;

typedef struct TransactionNode{
    Transaction transaction;
    int alreadyExploredExitWays[MAX_STATE_CHANNEL];
    struct TransactionNode *nextTransaction;
}TransactionNode;

//PACCHETTO UTILIZZATO PER LA COMUNICAZIONE IN FASE DI APERTURA DI UN CANALE
typedef struct ConnectionRequest{
    int confirmed;
    int usedValue; //valore proposto per la creazione del canale;
    PeerInfo info;
}ConnectionRequest;
//------------------------------------------------//


StateChannel* myChannels; 
PeerToReach* myPeerICanReach[HASH_TABLE_SIZE]; //Array di puntatori a PeerToReach (HASH TABLE CHE GESTISCE LE COLLISIONI CON UNA LISTA CONCATENATA)
TransactionNode * temporaryTransactions[HASH_TABLE_SIZE]; //Array di puntatori a TransactionNode (HASH TABLE CHE GESTISCE LE COLLISIONI CON UNA LISTA CONCATENATA)
PeerInfo myInfo;

//MUTEX PER LA MUTUA ESCLUSIONE E ACCESSO AI DATI
pthread_mutex_t mutexPeerICanReach;
pthread_mutex_t mutexMyChannels;
//-------------------------------------------//

char serverIp[16]; 
int myBalance; //Bilancio effettivo del peer
int lastTransactionId; //Id attuale della prossima transazione 

//FUNZIONI HASHTABLE DEI PEER DA RAGGIUNGERE
int hashCodePTR(int idPeerToReach);
PeerToReach* searchPeer(int idPeerToReach, PeerToReach* hashArray[]);
PeerToReach* insertPeer(int idPeerToReach, PeerToReach* hashArray[]);
int deletePeer(int idPeerToReach, PeerToReach* hashArray[]);
//------------------------------------------------//

//FUNZIONI HASHTABLE PER LE TRANSACTIONS TEMPORANEE
TransactionNode* searchTransaction(int idTransKey, int idSenderPeerKey, TransactionNode* hashArray[]);
void insertTransaction(TransactionNode* transactionToInsert, TransactionNode* hashArray[]);
void deleteTransaction(int idTransKey, int idSenderPeerKey, TransactionNode* hashArray[]);
void sendATransaction(int idPeer, Transaction transaction);
//------------------------------------------------//


//FUNZIONI GESTIONE LISTA CONCATENATA
void insertIntoList(StateChannel**head,StateChannel*newStateChannel);
void deleteFromList(StateChannel**head,int stateChannelId);
StateChannel*searchIntoList(StateChannel*head,int stateChannelId);
void printMyStateChannles();
//------------------------------------------------//

//FUNZIONI PER LA GESTIONE DEI CANALI
void closeStateChannel(int idStateChannelToDelete);
void closeStateChannelRequest(ConnectionRequest connectionRequest);
int openStateChannel();
void openChannelRequest(int clientFd, char clientIp[]);
//---------------------------------------------------//

int insertInput();
void humanInteraction(void* args);

//FUNZIONI E PROCEDURE PER L'ALGORITMO DI ROUTING
void updatePeersToReach(void *args);
void updateHashTablePeersToReach(int *response, int receivedSize, int idStateChannel);
void sendPeersToReach(void *args);
void getPeersICanReach(int* arrayToSend, int* sizeArray);
//----------------------------------------------------//

//FUNZIONI E PROCEDURE PER LA GESTIONE DELLE TRANSAZIONI
void transactionListener (void*args);
void sendATransaction(int idPeer, Transaction transaction);
void transactionHandler(void * args);
void initializeTransaction();
int searchAnExitWay(Transaction tempTransaction, TransactionNode *tempTransactionNode, PeerToReach *tempPeerToReach);
//----------------------------------------------------//


//FUNZIONI WRAPPER
int Socket(int domain,int type,int protocol);
int Connect(int sockfd, const struct sockaddr *addr,socklen_t addrlen);
void Listen(int sockfd, int backlog);
void Bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen);
int Accept(int sockfd, struct sockaddr *clientaddr, socklen_t *addr_dim);
ssize_t FullRead(int fd, void *buf, size_t count);
ssize_t FullWrite(int fd, const void *buf, size_t count);
//------------------------------------------------//

//FUNZIONI COMUNICAZIONE SERVER
void notifyConnection();
PeerInfo getPeerInfo(int idPeer);
//------------------------------------------------//

#endif // HEADER_H_INCLUDED