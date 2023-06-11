#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

// Maximum outstanding connection requests
#define MAXPENDING 7

// mutex
pthread_mutex_t mtx;
// all sectors where pirates are searching
int* areas;
// flag if treasure found
int wasFound = 0;
// pointer in pirate
unsigned short* pirates;
// quantity of pirates
int quantityOfPirates;
// index of the current pirate
int curPirate = 0;

// structure with pirates' arguments
typedef struct piratesArgs {
    int sock;
    int areas;
    struct sockaddr address;
} piratesArgs;

// function of checking arguments on right quantity.
void checkArguments(int quantity, char* text, char* ip) {
    if (quantity != 7) {
        fprintf(stderr, text, ip);
        exit(1);
    }
}

// process which simulate process of searching the treasure
void *piratesProcess(void* args) {
    // pirate socket
    int sock;
    struct sockaddr_in address;
    struct sockaddr_in curPir;
    pthread_detach(pthread_self());
    // receiving of arguments
    sock = ((piratesArgs*)args)->sock;
    int numOfAreas = ((piratesArgs*)args)->areas;
    // zeroing of args
    free(args);
    // client length
    unsigned int pl = sizeof(address);
    int given[2];
    int taken[2];
    int found = 0;
    int currentArea = -1;
    recvfrom(sock, taken, sizeof(taken), 0, (struct sockaddr*) &address, &pl);
    pthread_mutex_lock(&mtx);
    for (int i = 0; i < quantityOfPirates; i++) {
        if (pirates[i] == address.sin_port) {
            pthread_mutex_unlock(&mtx);
            goto phase;
        } else {
            pirates[curPirate++] = address.sin_port;
            pthread_mutex_unlock(&mtx);
            break;
        }
    }
    curPir = address;
    printf("Pirate %s connected\n", inet_ntoa(address.sin_addr));
    // endless loop
    while (1) {
        // lock the mutex
        pthread_mutex_lock(&mtx);
        //traversal of the areas
        for (int i = 0; i < numOfAreas; i++) {
            if (areas[i] != 1) {
                currentArea = i;
                if (areas[i] == 2) {
                    found = 1;
                } else {
                    found = 0;
                }
                areas[i] = 1;
                break;
            }
        }
        // unlock the mutex
        pthread_mutex_unlock(&mtx);
        given[0] = currentArea;
        given[1] = found;
        // sending to server the answer
        printf("SENDTO %s:%d\n", inet_ntoa(curPir.sin_addr), curPir.sin_port);
        sendto(sock, given, sizeof(given), 0, (struct sockaddr*) &curPir, sizeof(curPir));
        if (currentArea == -1) {
            break;
        }
        // receiving from server
        recvfrom(sock, taken, sizeof(taken), 0, (struct sockaddr*) &address, &pl);
    phase:
        // lock the mutex again
        pthread_mutex_lock(&mtx);
        // condition when we found the treasure
        if (taken[0] == 1) {
            if (!wasFound) {
                printf("Treasure was found!\n");
                wasFound = 1;
            }
            for (int i = 0; i < numOfAreas; i++) {
                areas[i] = 1;
            }
        }
        // unlock the mutex
        pthread_mutex_unlock(&mtx);
        currentArea = -1;
    }
}

int main(int argc, char *argv[]) {
    // echo server port
    unsigned short port;
    // server socket
    int ss;
    // client socket
    int cs;
    // client length
    int cl;
    // echo client address
    struct sockaddr_in c_address;
    // echo server address
    struct sockaddr_in s_address;
    pthread_t IDofProcess;
    // initializing of the mutex
    pthread_mutex_init(&mtx, NULL);
    // multicast
    int mc_socket;
    struct sockaddr_in mc_address;
    char *mc_ip;
    unsigned short mc_port;
    checkArguments(argc, "Usage:  %s <Multicast address> <Multicast port> <Client port> <Listener port> <Number of areas> <Area with treasure> <Quantity of pirates>\n", argv[0]);
    // receiving the arguments
    mc_ip = argv[1];
    mc_port = atoi(argv[2]);
    port = atoi(argv[3]);
    int numOfAreas = atoi(argv[4]);
    int areaWithTreasure = atoi(argv[5]) - 1;
    quantityOfPirates = atoi(argv[6]);
    // allocation of memory
    areas = (int*) malloc(sizeof(int) * numOfAreas);
    pirates = (unsigned short*) malloc(sizeof(unsigned short) * quantityOfPirates);
    for (int i = 0; i < numOfAreas; i++) {
        areas[i] = 0;
    }
    for (int i = 0; i < quantityOfPirates; i++) {
        pirates[i] = -1;
    }
    areas[areaWithTreasure] = 2;
    // creating of the new server socket using UDP
    ss = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    // checking of socket
    if (ss < 0) {
        perror("Process of socket was failed.");
        exit(1);
    }
    // create the server address structure
    memset(&s_address, 0, sizeof(s_address));
    s_address.sin_family = AF_INET;
    s_address.sin_addr.s_addr = htonl(INADDR_ANY);
    s_address.sin_port = htons(port);
    // establish the binding to the echo server
    if (bind(ss, (struct sockaddr *) &s_address, sizeof(s_address)) < 0) {
        perror("Process of bind was failed.");
        exit(1);
    }
    // creating of the new multicast socket using UDP
    mc_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    // checking of socket
    if (mc_socket < 0) {
        perror("Process of socket was failed.");
        exit(1);
    }

    int mc_ttl = 1;
    if (setsockopt(mc_socket, IPPROTO_IP, IP_MULTICAST_TTL, (void *) &mc_ttl, sizeof(mc_ttl)) < 0) {
        perror("Process of setsockopt was failed.");
        exit(1);
    }
    // create the multicast address structure
    memset(&mc_address, 0, sizeof(mc_address));
    mc_address.sin_family = AF_INET;
    mc_address.sin_addr.s_addr = inet_addr(mc_ip);
    mc_address.sin_port = htons(mc_port);
    printf("Open multicast socket on %s:%d\n", inet_ntoa(mc_address.sin_addr), mc_port);

    for (int i = 0; i < quantityOfPirates; i++) {
        piratesArgs *args = (piratesArgs *) malloc(sizeof(piratesArgs));
        args->sock = ss;
        args->areas = numOfAreas;
        if (pthread_create(&IDofProcess, NULL, piratesProcess, (void*) args) != 0) {
            perror("Process of creating of pthread was failed.");
            exit(1);
        }
    }
    while(!wasFound) {
        sendto(mc_socket, areas, sizeof(int) * numOfAreas, 0, (struct sockaddr *) &mc_address, sizeof(mc_address));
        sleep(1);
    }
    areas[0] = -1;
    sendto(mc_socket, areas, sizeof(int) * numOfAreas, 0, (struct sockaddr *) &mc_address, sizeof(mc_address));
    sleep(5);
    free(pirates);
    free(areas);
    pthread_mutex_destroy(&mtx);
}
