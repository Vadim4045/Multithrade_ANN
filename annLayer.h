#ifndef H_LAYER
#define H_LAYER

#define MIN_PER_THREAD 20

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

typedef struct _annLayer
{
    unsigned int count;
    double alfa;
    struct _annLayer* next;
    double* content;
    double* fallacy;
    double** weights;
    int threads;
    struct _thread_data* threadData;
    pthread_t* threadsArr;
} annLayer;

typedef struct _thread_data{
    annLayer* layer;
    int start;
    int end;
    double mu;
}thread_data;

annLayer* newLayer(int, double, int, int);
annLayer* layerMakeContinue(annLayer*, annLayer*);
void layerFP(annLayer*);
void layerBP(annLayer*, double);
int freeLayer(annLayer*);
void randomWeights(annLayer*);

#endif //H_LAYER