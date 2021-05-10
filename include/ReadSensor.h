#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <semaphore.h>
#include <stdio.h>

sem_t semaphoreWaterSupply;
int waterSupplyFieldCapacity;

void readSensorDataFrom(int FD, int pidToHandle, int fieldNo, int minCapacity, int maxCapacity, int *currStatus);