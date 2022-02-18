#ifndef _MAIN
#define _MAIN

#define BUFF_BIG 100
#define BUFF_SMALL 50


#include <stdio.h>
#include <stdlib.h>
#include <mcheck.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "simpleANN.h"
#include "dataImport.h"

typedef struct _config
{
    int layers;
    int layers_counts[5];
    double alfa;
    double mu;
    char data[BUFF_SMALL];
    int data_len;
    char weights[BUFF_SMALL];
    int epochs;
    int mode;
    int threads;
}config;

config* parseCommand(int, char**);
void printDataSet(int, int, double*);
//void printConfig(config*);

extern int errno;

int main(int argc, char *argv[]){
    Ann* ann;
    double** data;
    time_t start_t, end_t;
    double diff_t;
    config* conf;
    int i, num1, num2, good=0;

    mtrace();

    if(argc==2){
        conf = parseCommand(argc, argv);
    }else{
        printf("Not enough parameters.\n");
        exit(0);
    }
    if (conf == NULL){
        printf("Config not readed.\n");
        exit(0);
    }

    

    data = getData(conf->data, &conf->data_len);
    if(data == NULL){
        perror("Dataset reading error");
        return(errno);
    }

    ann = newSimpleANN(conf->layers, conf->layers_counts, conf->alfa, conf->weights, conf->threads);

    if(conf->mode==0){
        for(i = 1; i <= conf->epochs; i++){
            time(&start_t);

            good = simpleAnnLearn(ann,data, conf->data_len, conf->mu);

            time(&end_t);
            diff_t = difftime(end_t, start_t);

            printf("Step - %d from %d: %d/%d on %f sec.\n", i, conf->epochs, conf->data_len, good, diff_t);
        }
    }else if(conf->mode==1){
        time(&start_t);

        for(i=0;i<conf->data_len;i++){
            num1 = simpleAnnGo(ann, data[i], &data[i][conf->layers_counts[0]]);
            num2 = checkNumber(ann, &data[i][conf->layers_counts[0]]);
            if(num1!=num2){
                //printf(" %d/%d ", num1, num2);
                //printDataSet(28, 28, data[i]);
            }else good++;
            
        }

        time(&end_t);
        diff_t = difftime(end_t, start_t);
        printf("\n%d numbers from %d on %f sec.\n",good, conf->data_len, diff_t);
    }
    

    freeSimpleANN(ann);
    freeDataMemory(data, conf->data_len);
    free(conf);

    exit(0);
}

config* parseCommand(int argc, char *argv[]){

    FILE *file;
    int i, j, count=0;
    char tmp[BUFF_BIG], tmp1[BUFF_SMALL], tmp2[BUFF_SMALL];
    config* conf = (config*) calloc(1, sizeof(config));
    if(conf == NULL){
        perror("Memory allocation error");
        return NULL;
    }


    file = fopen(argv[1],"r");
    if(file==NULL){
        perror("Config file open error");
        return NULL;
    }

    while (1)
    {
        i=0;
        while(1) 
        {
            tmp[i] = (char)fgetc(file);

            if(tmp[i] != EOF && tmp[i] != '\n'){
                if(tmp[i] != ' '){
                    i++;
                }
            }else{
                break;
            }
        };

        if(tmp[i] == EOF){
            if(count==9+conf->layers){
                return conf;
            }else{
                free(conf);
                return NULL;
            }
        }

        if(tmp[0]=='#' || tmp[0] == '\n') continue;

        if(tmp[i] == '\n') tmp[i] = '\0';

        strcpy(tmp1,strtok(tmp, "="));
        strcpy(tmp2,strtok(NULL, "="));
        
        if(tmp1[0] >='0' && tmp1[0] <= '9'){
            conf->layers_counts[atoi(tmp1)] = atoi(tmp2);
            count++;
        }
        
        if(!strcmp(tmp1, "mu")){
            conf->mu = atof(tmp2);
            count++;
        }
        
        if(!strcmp(tmp1, "dataLen")){
            conf->data_len = atoi(tmp2);
            count++;
        }
        
        if(!strcmp(tmp1, "data")){
            strcpy(conf->data, tmp2);
            count++;
        }
        
        if(!strcmp(tmp1, "epochs")){
            conf->epochs = atoi(tmp2);
            count++;
        }
        
        if(!strcmp(tmp1, "threads")){
            conf->threads = atoi(tmp2);
            count++;
        }

        if(!strcmp(tmp1, "mode")){
            conf->mode = atoi(tmp2);
            count++;
        }

        if(!strcmp(tmp1, "weights")){
            strcpy(conf->weights, tmp2);
            count++;
        }
        
        if(!strcmp(tmp1, "layers")){
            conf->layers = atoi(tmp2);
            count++;
        }
        
        if(!strcmp(tmp1, "alfa")){
            conf->alfa = atof(tmp2);
            count++;
        } 
    };
}

void printDataSet(int columns, int rows, double* data){
    int j, k;

    printf("\n");

    for(j=0;j<rows;j++){
        for(k=0;k<columns;k++){
            if(data[j*columns+k]>0.5){
                printf(".");
            }else{
                printf(" ");
            }
        }
        printf("\n");
    }

    printf("\n");
}

/* void printConfig(config* conf){
    int i;

    printf("\nThere are %d layers:\n", conf->layers);
    for(i=0;i<5;i++){
        printf("%d. %d neurons\n", i, conf->layers_counts[i]);
    }
    printf("Dataset file is %s\n", conf->data);
    printf("Data array length is %d\n", conf->data_len);
    printf("Weights file is %s\n", conf->weights);
    printf("Alfa is %f\n", conf->alfa);
    printf("Mu is %f\n", conf->mu);
    printf("Lern epochs: %d\n", conf->epochs);
} */

#endif // _MAIN