#include <ReadSensor.h>

void readSensorDataFrom(int FD, int pidToHandle, int fieldNo, int minCapacity, int maxCapacity, int *currStatus) {
    int ground_water_level = 0;
    int idFlag = 1;
    int randomNumber;
    int semValue;
    char *waterLevel = (char *) malloc(3 * sizeof(char));

    int fd = open("WaterSupply.log", O_CREAT | O_APPEND|O_RDWR);
    char buf[1024];

    while (1) {
        if (ground_water_level == 0 && idFlag == 1) {//initial motor start condition
            idFlag = 0;
            sem_wait(&semaphoreWaterSupply);
            sem_getvalue(&semaphoreWaterSupply, &semValue);
            if (semValue == (waterSupplyFieldCapacity - 1)) {
                sprintf(buf, "\n<<<<<<<<<<start motor for water supply>>>>>>>>>>\n");
                write(fd, buf, strlen(buf));
            }
            sprintf(buf, "\tstart passing water to field:%d\n", fieldNo);
            write(fd, buf, strlen(buf));

            kill(pidToHandle, SIGCONT);//process resume
        } else if (0 < read(FD, waterLevel, 3)) {
            char *water = (char *) malloc(3 * sizeof(char));;
            strncpy(water, waterLevel, 3);
            ground_water_level = atoi(water);
            *currStatus = ground_water_level;
            if (ground_water_level <= minCapacity && idFlag) {
                idFlag = 0;
                sem_wait(&semaphoreWaterSupply);
                sem_getvalue(&semaphoreWaterSupply, &semValue);
                if (semValue == (waterSupplyFieldCapacity - 1)) {
                    sprintf(buf, "\n<<<<<<<<<<start motor for water supply>>>>>>>>>>\n");
                    write(fd, buf, strlen(buf));
                }
                sprintf(buf, "\tstart passing water to field:%d\n", fieldNo);
                write(fd, buf, strlen(buf));

                kill(pidToHandle, SIGCONT);//process resume
            } else if (ground_water_level >= maxCapacity && !idFlag) {
                idFlag = 1;
                sprintf(buf, "\tstop passing water to field:%d\n", fieldNo);
                write(fd, buf, strlen(buf));
                sem_post(&semaphoreWaterSupply);
                sem_getvalue(&semaphoreWaterSupply, &semValue);
                if (semValue == waterSupplyFieldCapacity) {
                    sprintf(buf, "\n<<<<<<<<<<stop motor for water supply>>>>>>>>>>\n");
                    write(fd, buf, strlen(buf));
                }
                kill(pidToHandle, SIGCONT); //process resume
            }
        }
    }
}