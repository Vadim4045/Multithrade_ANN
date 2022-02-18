#ifndef H_ANN
#define H_ANN
#define NELEMS(x)  (int)(sizeof(x) / sizeof((x)[0]))

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "annLayer.h"

typedef struct simpleANN
{
    int layersCount;
    int* configArr;
    annLayer** innerLayers;
    const char* weights_folder;
    int epoch;
} Ann;

Ann* newSimpleANN(int, int*, double, const char*, int);
int simpleAnnGo(Ann*, double*, double*);
int simpleAnnLearn(Ann*, double**, int, double);
int checkNumber(Ann*, double*);
int freeSimpleANN(Ann*);

#endif //H_ANN