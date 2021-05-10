#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {

    int ground_water_level = 0;
    int randomNumber;
    int minWater = atoi(argv[1]);
    int maxWater = atoi(argv[2]);
    int FD = atoi(argv[3]);

    char *waterLevel = (char *) malloc(4 * sizeof(char));
    srand(time(NULL));
    while (1) {

        raise(SIGSTOP); //thread suspend

        while (ground_water_level < maxWater) {
            randomNumber = rand() % 10 + 1;
            sleep(randomNumber);
            ground_water_level += randomNumber;

            sprintf(waterLevel, "%03d\n", ground_water_level);
            write(FD, waterLevel, strlen(waterLevel));
            if (ground_water_level >= maxWater) {
                break;
            }
        }

        raise(SIGSTOP); //thread suspend

        while (ground_water_level > minWater) {

            randomNumber = rand() % 10 + 1;
            sleep(randomNumber);
            ground_water_level -= randomNumber;

            sprintf(waterLevel, "%03d\n", ground_water_level);
            write(FD, waterLevel, strlen(waterLevel));
            if (ground_water_level <= minWater) {
                break;
            }
        }
    }
}