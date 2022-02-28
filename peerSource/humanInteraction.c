#include "header.h"

void exit_handler(int segnale_ricevuto);


void humanInteraction(void* args){

    Transaction transactionRequest;
    int listenCreateChannelFd,clientFd,enable=1,idPeer,usedValues,idPeerTransaction,i=0;
    int input,checkInput;
    fd_set readSet;//FD_SET PER ASCOLTARE CHI VUOLE CREARE UN CANALE CON ME
    
    int idStateChannelToDelete;

    struct sockaddr_in infoClient;
    socklen_t size_infoClient=(socklen_t)sizeof(struct sockaddr_in);
    char clientIp[16];
    //CREO LA SOCKET PER LA CREAZIONE DEGLI STATECHANNEL
    struct sockaddr_in serverAddr;
    listenCreateChannelFd=Socket(AF_INET,SOCK_STREAM,0);

    serverAddr.sin_port=htons(myInfo.channelPort);
    serverAddr.sin_family=AF_INET;
    serverAddr.sin_addr.s_addr=htonl(INADDR_ANY);

    setsockopt(listenCreateChannelFd, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &enable, sizeof(int));

    Bind(listenCreateChannelFd,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
    Listen(listenCreateChannelFd,1000);
    //------------------------------------------------//


    int fd_disponibili;
    FD_ZERO(&readSet);
    int max_fd=listenCreateChannelFd+1;



    while(1)
    {
        printf("BILANCIO ATTUALE: %d\n", myBalance);
        printf("|--- IN ASCOLTO ---|\n1. Chiudi uno state channel.\n2. Effettua transazione\n3. Vedi gli state channel aperti.\n|----------------|\n");

        FD_SET(listenCreateChannelFd,&readSet);
        FD_SET(STDIN_FILENO,&readSet);


        while(((fd_disponibili=select(listenCreateChannelFd+1,&readSet,NULL,NULL,NULL)) < 0)&&(errno==EINTR));

        //leggo da standard input
        if (FD_ISSET(0,&readSet)){

            
            input=insertInput();

            switch(input)
            {
                case 1:
                    printf("Inserisci l'id del peer con cui vuoi chiudere lo State channel: ");
                    idStateChannelToDelete=insertInput();
                    closeStateChannel(idStateChannelToDelete);

                    break;
                case 2:
                    initializeTransaction();
                    break;
                case 3:
                    printMyStateChannles();
                    break;
                default:
                    printf("\nNon hai inserito una scelta valida.");
                    break;
             }
        }//FINE CREAZIONE STATE CHANNEL

        //accetto un client
        if (FD_ISSET(listenCreateChannelFd,&readSet)){

            //Accetto un client che vuole creare uno stateChannel
            clientFd=Accept(listenCreateChannelFd,(struct sockaddr*)&infoClient,&size_infoClient);
            inet_ntop(AF_INET, &infoClient.sin_addr.s_addr, clientIp,sizeof(clientIp));
            openChannelRequest(clientFd, clientIp);

            close(clientFd);
        }//FINE RICHIESTA DA ALTRO PEER

    }
}