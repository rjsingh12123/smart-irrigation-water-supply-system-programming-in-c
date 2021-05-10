#include "../include/ReadSensor.h"
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <semaphore.h>
#include <time.h>
#include <netdb.h>

#define    READ    0
#define    WRITE    1
#define    MAXLINE     8192

int numberOfWaterSensor = 0;
int *waterSensorPid;
int *minArr;
int *maxArr;
int *currStatusArr = NULL;
pthread_t pthreadMain;
pthread_t *pthreadWaterSensorRead;

struct sensorParams {
    int field;
    int pidToHandle;
    int minWater, maxWater;
    int fd;
    int *currStatus;
};

void *threadWaterSensorReadHandler(void *arg) {
    struct sensorParams *pa = arg;
    srand(time(NULL));
    sleep(rand() % 10);
    readSensorDataFrom((*pa).fd, ((*pa).pidToHandle), ((*pa).field), ((*pa).minWater), ((*pa).maxWater),
                       ((*pa).currStatus));
}

void *threadMainSystemdHandler(void *arg) {
    waterSensorPid = calloc(numberOfWaterSensor, sizeof(int));
    currStatusArr = calloc(numberOfWaterSensor, sizeof(int));
    struct sensorParams pa[numberOfWaterSensor];
    sem_init(&semaphoreWaterSupply, 0, waterSupplyFieldCapacity);
    pthreadWaterSensorRead = calloc(numberOfWaterSensor, sizeof(pthread_t));
    int waterSensorFD[numberOfWaterSensor][2];

    for (size_t i = 0; i < numberOfWaterSensor; i++) {
        char *args[5];
        pipe(waterSensorFD[i]);
        waterSensorPid[i] = fork();
        if (waterSensorPid[i] == 0) {
            close(waterSensorFD[i][READ]);

            args[0] = "./waterLevelSensor";
            args[1] = malloc(3 * sizeof(char));
            sprintf(args[1], "%d", minArr[i]);
            args[2] = malloc(3 * sizeof(char));
            sprintf(args[2], "%d", maxArr[i]);
            args[3] = malloc(3 * sizeof(char));
            sprintf(args[3], "%d", waterSensorFD[i][WRITE]);
            args[4] = NULL;

            execvp("./waterLevelSensor", args);
            printf("error reading sensor data");
            close(waterSensorFD[i][WRITE]);
        } else {
            close(waterSensorFD[i][WRITE]);
                free(args[1]);
                free(args[2]);
                free(args[3]);
        }

    }

    for (size_t i = 0; i < numberOfWaterSensor; i++) {
        pa[i].pidToHandle = waterSensorPid[i];
        pa[i].field = (int) (i + 1);
        pa[i].minWater = minArr[i];
        pa[i].maxWater = maxArr[i];
        pa[i].fd = waterSensorFD[i][READ];
        pa[i].currStatus = &currStatusArr[i];
        pthread_create(&pthreadWaterSensorRead[i], NULL, threadWaterSensorReadHandler,
                       (void *) &pa[i]);
    }

    for (size_t i = 0; i < numberOfWaterSensor; i++) {
        pthread_join(pthreadWaterSensorRead[i], NULL);
        close(waterSensorFD[i][READ]);
    }

    sem_destroy(&semaphoreWaterSupply);
}

void ServerStartSystem(int connfd) {
    char buf[MAXLINE];
    sprintf(buf, "%d\n", numberOfWaterSensor);
    write(connfd, buf, strlen(buf));
    if (numberOfWaterSensor == 0) {
        minArr = calloc(numberOfWaterSensor, sizeof(int));
        maxArr = calloc(numberOfWaterSensor, sizeof(int));
        read(connfd, buf, MAXLINE);
        sscanf(buf, "%d", &numberOfWaterSensor);
        printf("\nno of fields:\t%d\n", numberOfWaterSensor);
        read(connfd, buf, MAXLINE);
        sscanf(buf, "%d", &waterSupplyFieldCapacity);
        printf("\nSimultaneously Supply:\t%d\n", waterSupplyFieldCapacity);

        for (size_t i = 0; i < numberOfWaterSensor; i++) {
            read(connfd, buf, MAXLINE);
            sscanf(buf, "%d", &minArr[i]);
            printf("\nMin %% for field-%d:\t%d\n", (int) i + 1, minArr[i]);
            read(connfd, buf, MAXLINE);
            sscanf(buf, "%d", &maxArr[i]);
            printf("\nMax %% for field-%d:\t%d\n", (int) i + 1, maxArr[i]);
        }

        pthread_create(&pthreadMain, NULL, threadMainSystemdHandler, NULL);
        printf("System has been stated successfully\n");
        printf("Water Supply Logs are written inside WaterSupply.log file\n");
    } else {
        sprintf(buf, "System is already running !\n");
        write(connfd, buf, strlen(buf));
    }
}


void ServerCurrentStatus(int connfd) {
    char buf[MAXLINE];
    char temp[MAXLINE];
    printf("\n<<<<<<<<<<<<<Printing Status>>>>>>>>>>>>>>>>>\n");
    strcpy(buf, "\n<<<<<<<<<<<<<Printing Status>>>>>>>>>>>>>>>>>\n");

    for (size_t i = 0; i < numberOfWaterSensor; i++) {
        printf("Field %d:\t%d\n", (int) i + 1, currStatusArr[i]);
        sprintf(temp, "Field %d:\t%d\n", (int) i + 1, currStatusArr[i]);
        strcat(buf, temp);
    }

    sprintf(temp, "\nCheck WaterSupply.log file for more details\n");
    strcat(buf, temp);
    write(connfd, buf, strlen(buf));
}

void ServerStopSystem(int connfd) {
    char buf[MAXLINE];

        //removing assigned threads and processes
    for (size_t i = 0; i < numberOfWaterSensor; ++i) {
        pthread_cancel(pthreadWaterSensorRead[i]);
        kill(waterSensorPid[i],SIGKILL);
    }
    pthread_cancel(pthreadMain);
    free(waterSensorPid);
    free(minArr);
    free(maxArr);
    free(currStatusArr);
    free(pthreadWaterSensorRead);

    numberOfWaterSensor = 0;

    remove("./WaterSupply.log");
    printf("\n<<<<<<<<<<<<<Terminating System>>>>>>>>>>>>>>>>>\n");
    sprintf(buf, "\n<<<<<<<<<<<<<Terminating System>>>>>>>>>>>>>>>>>\n");
    write(connfd, buf, MAXLINE);
}


int open_listenfd(char *port) {
    struct addrinfo hints, *listp, *p;
    int listenfd, optval = 1;
    char host[MAXLINE], service[MAXLINE];
    int flags;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE |
                     AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;
    getaddrinfo(NULL, port, &hints, &listp);

    for (p = listp; p; p = p->ai_next) {

        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;  /* Socket failed */

        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *) &optval, sizeof(int));

        flags = NI_NUMERICHOST |
                NI_NUMERICSERV;
        getnameinfo(p->ai_addr, p->ai_addrlen, host, MAXLINE, service, MAXLINE, flags);
        printf("host:%s, service:%s\n", host, service);

        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break; /* Success */
        close(listenfd); /* Bind failed */
    }

    /* Clean up */
    freeaddrinfo(listp);
    if (!p)
        return -1;

    if (listen(listenfd, 1) < 0) {
        close(listenfd);
        return -1;
    }
    return listenfd;
}

void echo(int connfd) {
    size_t n;
    char buf[MAXLINE];
    while ((n = read(connfd, buf, MAXLINE)) != 0) {
        int serviceOption;
        serviceOption = (int) buf[8];
        serviceOption -= 48;
        switch (serviceOption) {
            case 1:
                printf("\nSelected Option : Start System\n");
                ServerStartSystem(connfd);
                break;
            case 2:
                printf("\nSelected Option : Current Status\n");
                ServerCurrentStatus(connfd);
                break;
            case 3:
                printf("\nSelected Option : Terminate System\n");
                ServerStopSystem(connfd);
        }
    }
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];
    listenfd = open_listenfd(argv[1]);
    while (1) {
        printf("Waiting for Client please wait\n");
        clientlen = sizeof(struct sockaddr_storage); /* Important! */
        connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
        getnameinfo((struct sockaddr *) &clientaddr, clientlen,
                    client_hostname, MAXLINE, client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        echo(connfd);
        printf("closing connection please wait\n");
        close(connfd);
    }
    exit(0);
}