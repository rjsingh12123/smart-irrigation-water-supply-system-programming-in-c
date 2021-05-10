#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>

#define    MAXLINE     8192  /* Max text line length */
int numberOfWaterSensor =0;

int open_clientfd(char *hostname, char *port) {
    int clientfd;
    struct addrinfo hints, *listp, *p;
    char host[MAXLINE], service[MAXLINE];
    int flags;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_flags |= AI_ADDRCONFIG;
    getaddrinfo(hostname, port, &hints, &listp);

    for (p = listp; p; p = p->ai_next) {

        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue; /* Socket failed */

        flags = NI_NUMERICHOST |
                NI_NUMERICSERV;
        getnameinfo(p->ai_addr, p->ai_addrlen, host, MAXLINE, service, MAXLINE, flags);
        printf("host:%s, service:%s\n", host, service);

        /* Connecting server */
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1) {
            printf("Connected to server %s at port %s\n", host, service);
            break; /* Success */
        }
        close(clientfd); /* Connect failed */
    }

    freeaddrinfo(listp);
    if (!p) /* All failed */
        return -1;
    else    /* The last connect succeeded */
        return clientfd;
}

void startSystemClient(int clientfd) {
    char buf[MAXLINE];
    read(clientfd, buf, 2);
    sscanf(buf,"%d",&numberOfWaterSensor);
    if (numberOfWaterSensor==0) {
        printf("\nEnter no of fields\t");
        int waterSupplyFieldCapacity, min, max;
        scanf("%d", &numberOfWaterSensor);
        sprintf(buf, "%d", numberOfWaterSensor);
        write(clientfd, buf, strlen(buf));
        printf("\nEnter no of fields in which water \ncan be supplied simultaneously\t");
        scanf("%d", &waterSupplyFieldCapacity);
        sprintf(buf, "%d", waterSupplyFieldCapacity);
        write(clientfd, buf, strlen(buf));

        for (size_t i = 0; i < numberOfWaterSensor; i++) {
            printf("\nEnter Min %% for field: %d\t", (int) i + 1);
            scanf("%d", &min);
            sprintf(buf, "%d", min);
            write(clientfd, buf, strlen(buf));
            printf("\nEnter Max %% for field: %d\t", (int) i + 1);
            scanf("%d", &max);
            sprintf(buf, "%d", max);
            write(clientfd, buf, strlen(buf));
        }
    } else {
        read(clientfd, buf, MAXLINE);
        printf("%s", buf);
    }
}

void currentStatusClient(int clientfd) {
    char buf[MAXLINE];
    read(clientfd, buf, MAXLINE);
    printf("%s", buf);
}

void stopSystemClient(int clientfd) {
    char buf[MAXLINE];
    read(clientfd, buf, MAXLINE);
    printf("%s", buf);
}

int main(int argc, char **argv) {
    int clientfd;
    char *host, *port, buf[MAXLINE];
    host = argv[1];
    port = argv[2];
    clientfd = open_clientfd(host, port);
    int serviceOption;
    while (1) {
        printf("\n===============\n");
        printf("Select Service \n");
        printf("===============\n");
        printf("1. Start System \n");
        printf("2. Current Status \n");
        printf("3. Terminate System \n");
        printf("4. EXIT\n");
        scanf("%d", &serviceOption);
        sprintf(buf, "option: %d", serviceOption);
        if(serviceOption==4)
            break;
        write(clientfd, buf, strlen(buf));
        switch (serviceOption) {
            case 1:
                startSystemClient(clientfd);
                break;
            case 2:
                currentStatusClient(clientfd);
                break;
            case 3:
                stopSystemClient(clientfd);
                break;
        }
    }
    close(clientfd);
    exit(0);
}
