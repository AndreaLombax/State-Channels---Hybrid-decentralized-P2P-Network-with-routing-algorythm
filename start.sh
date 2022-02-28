#!/bin/bash
cd peerSource/
gcc peer.c ON_HashTable.c serverCommunication.c wrapper.c openStateChannel.c ListFunctions.c updatePeersToReach.c header.h humanInteraction.c initializeTransaction.c TR_HashTable.c transaction.c closeStateChannel.c -o peer -lpthread
cd ..
cd server/
gcc server.c headerServer.h utilFunctions.c wrapper.c -o server
./server