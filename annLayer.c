#include "annLayer.h"

double sigma(double, double);
double sigmaPrime(double, double);
void* fpThread(void*);
void* bpThread(void*);

annLayer* newLayer(int count, double alfa, int position, int threads){
    unsigned int i;

    annLayer* newLayer = (annLayer*) calloc(1, sizeof(annLayer));
    if (newLayer==NULL) {
        return NULL;
    }

    newLayer->count = count;
    newLayer->alfa=alfa;
    newLayer->threads = threads;

    newLayer->content = (double*) calloc(count, sizeof(double));
    if(newLayer->content == NULL){
        freeLayer(newLayer);
        return NULL;
    }

    if(position>0){
        newLayer->fallacy = (double*) calloc(count, sizeof(double));
        if(newLayer->fallacy == NULL){
            freeLayer(newLayer);
            return NULL;
        }
    }

    return newLayer;
}

annLayer* layerMakeContinue(annLayer* layer, annLayer* nextLayer){
    int i, per_thread;

    layer->next = nextLayer;

    layer->weights = (double**) calloc(nextLayer->count, sizeof(double*));
    if(layer->weights == NULL){
        freeLayer(layer);
        return NULL;
    }

    for(i=0;i<nextLayer->count;i++){
        layer->weights[i] = (double*) calloc(layer->count, sizeof(double));
        if(layer->weights[i]==NULL){
            freeLayer(layer);
            return NULL;
        }
    }

    per_thread = layer->next->count/layer->threads;
    while (layer->threads > 1 && per_thread < MIN_PER_THREAD){
        layer->threads--;
        per_thread = layer->next->count/layer->threads;
    }
    
    layer->threadData = (thread_data*) calloc(layer->threads, sizeof(thread_data));
    if(layer->threadData==NULL){
        freeLayer(layer);
        return NULL;
    }

    if(layer->threads>1){
        for(i=0;i<layer->threads;i++){
            layer->threadData[i].layer = layer;
            layer->threadData[i].start = i*per_thread;
            layer->threadData[i].end = (i+1)*per_thread;
        }
        layer->threadData[layer->threads-1].end = layer->next->count;

        layer->threadsArr = (pthread_t*) calloc(layer->threads, sizeof(pthread_t));
        if(layer->threadsArr==NULL){
            freeLayer(layer);
            return NULL;
        }
    }
    

    return layer;
}

void randomWeights(annLayer* layer){
    unsigned int i, j;

    srand(time(NULL));

    for(i=0;i<layer->next->count;i++){
        for(j=0;j<layer->count;j++){
            layer->weights[i][j] = (((float)rand()/(float)RAND_MAX)-0.5)*0.1;
        }
    }
}

void layerFP(annLayer* layer){
    int i, j;

    if(layer->threads>1){

        for(i=0;i<layer->threads;i++){
            pthread_create(&layer->threadsArr[i], NULL, fpThread, &layer->threadData[i]);
        }

        for(i=0;i<layer->threads;i++){
            pthread_join(layer->threadsArr[i], NULL);
        }

    }else{
        for(i=0;i<layer->next->count;i++){
            layer->next->content[i]=0;
            layer->next->fallacy[i] = 0;

            for(j=0;j<layer->count;j++){
                layer->next->content[i] += layer->content[j] * layer->weights[i][j];
            }

            layer->next->content[i] = sigma(layer->next->content[i], layer->alfa);       
        }
    }
    
}

void* fpThread(void* threadData){
    int i, j;

    thread_data* data = (thread_data*) threadData;

    for(i=data->start;i<data->end;i++){
        data->layer->next->content[i]=0;
        data->layer->next->fallacy[i] = 0;

        for(j=0;j<data->layer->count;j++){
            data->layer->next->content[i] += data->layer->content[j] * data->layer->weights[i][j];
        }

        data->layer->next->content[i] = sigma(data->layer->next->content[i], data->layer->alfa);       
    }

    return NULL;
}

void layerBP(annLayer* layer, double mu){
    unsigned int i,j;
   
    if(layer->fallacy != NULL){
        for(i=0;i<layer->next->count;i++){
            for(j=0;j<layer->count;j++){ 
                layer->fallacy[j] += layer->weights[i][j]*layer->next->fallacy[i];
            }
        }
    }

    if(layer->threads>1){

        for(i=0;i<layer->threads;i++){
            layer->threadData[i].mu = mu;
            
            pthread_create(&layer->threadsArr[i], NULL, bpThread, &layer->threadData[i]);
        }

        for(i=0;i<layer->threads;i++){
            pthread_join(layer->threadsArr[i], NULL);
        }

    }else{
        for(i=0;i<layer->next->count;i++){
            for(j=0;j<layer->count;j++){
                layer->weights[i][j] += mu*layer->next->fallacy[i]*sigmaPrime(layer->next->content[i], layer->alfa)*layer->content[j];
            }
        }
    }
    

}

void* bpThread(void* threadData){
    int i, j;

    thread_data* data = (thread_data*) threadData;

    for(i=data->start;i<data->end;i++){
        for(j=0;j<data->layer->count;j++){
            data->layer->weights[i][j] += data->mu*data->layer->next->fallacy[i]*sigmaPrime(data->layer->next->content[i], data->layer->alfa)*data->layer->content[j];
        }
    }

    return NULL;
}

int freeLayer(annLayer* layer){
    unsigned int i;

    if(layer == NULL){
        return 0;
    }

    if(layer->content != NULL){
        free(layer->content);
    }

    if(layer->fallacy != NULL){
        free(layer->fallacy);
    }

    if(layer->weights != NULL){
        for(i=0;i<layer->next->count;i++){
            if(layer->weights[i] != NULL){
                free(layer->weights[i]);
            }
            
        }

        free(layer->weights);
    }

    if(layer->threads>1){
        free(layer->threadData);
        free(layer->threadsArr);
    }
    
    free(layer);

    return 1;
}

inline double sigma(double num, double alfa){
    return num>0? alfa*(num+1):0.01*(num+1);//1.0/(1.0+exp(-alfa*num));
}

inline double sigmaPrime(double num, double alfa){
    return num>0? alfa:0.01*alfa;//alfa*num*(1.0-alfa*num);
}