#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "collection.h"


Tarefa initTarefa(int id, int pid, int status, char* command) {
    Tarefa new = malloc((sizeof(struct tarefa)));
    new->id = id;
    new->pid = pid;
    new->status = status;
    strcpy(new->command,command);
    return new;
}

TList* initialNode() {
    return malloc((sizeof(struct linkedlist)));
}

void addTarefa(TList* list, Tarefa tarefa) {

    while(*list) {
        list = &((*list)->next);
    }
    *list=(TList)malloc(sizeof(struct linkedlist));
    (*list)->tarefa = tarefa;
    (*list)->next = NULL;
}

int pop(TList* head) {
    int retval = -1;
    TList next_node = NULL;

    if (*head == NULL) {
        return -1;
    }

    next_node = (*head)->next;
    retval = (*head)->tarefa->id;
    free(*head);
    *head = next_node;

    return retval;
}


int remove_by_index(TList* head, int n) {
    int i = 0;
    int retval = -1;
    TList current = *head;
    TList temp_node = NULL;

    if (n == 0) {
        return pop(head);
    }

    for (i = 0; i < n-1; i++) {
        if (current->next == NULL) {
            return -1;
        }
        current = current->next;
    }

    temp_node = current->next;
    retval = temp_node->tarefa->id;
    current->next = temp_node->next;
    free(temp_node);

    return retval;
}

int remove_by_id(TList *l, int id){
    TList p;
    int t=-1;
    while(*l && (*l)->tarefa->id!=id)
        l=&((*l)->next);
    if(*l){
        p=(*l)->next;
        free(*l);
        *l=p;
        t=0;
    }
    return t;
}

Tarefa remove_by_pid(TList *l, int pid){
    TList p;
    Tarefa ret = NULL;
    int t=-1;
    while(*l && (*l)->tarefa->pid!=pid)
        l=&((*l)->next);
    if(*l){
        p=(*l)->next;
        ret = (*l)->tarefa;
        *l=p;
        t=0;
    }
    return ret;
}

int pid_of_id(TList *l, int id){
    TList p;
    int t=-1;
    while(*l && (*l)->tarefa->id!=id)
        l=&((*l)->next);
    if(*l){
        t = (*l)->tarefa->pid;
    }
    return t;
}


void imprimeL (TList x){
    while(x){
        printf("Tarefa #%d: pid - %d, status - %d, command - %s\n",x->tarefa->id,x->tarefa->pid,x->tarefa->status,x->tarefa->command);
        x=x->next;
    }
}

void imprimeT (Tarefa x){
    //printf("Tarefa #%d: pid - %d, status - %d, command - %s\n",x->id,x->pid,x->status,x->command);
    printf("Tarefa #%d: pid - %d, status - %d, command - %s\n",x->id,x->pid,x->status,x->command);
}

char* resOfStatus(int status) {
    char* ret;
    if(status == 0) {
        ret = strdup("concluida");
    } else if (status == 1) {
        ret = strdup("max execução");
    } else if (status == 2) {
        ret = strdup("max inactividade");
    } else {
        ret = strdup("terminada");
    }
    return ret;
}


void imprimeTStatus (Tarefa x){
    //printf("Tarefa #%d: pid - %d, status - %d, command - %s\n",x->id,x->pid,x->status,x->command);
    printf("#%d, %s: %s\n",x->id,resOfStatus(x->status),x->command);
}

char* stringTStatus (Tarefa x){
    //printf("Tarefa #%d: pid - %d, status - %d, command - %s\n",x->id,x->pid,x->status,x->command);
    //printf("#%d, %s: %s\n",x->id,resOfStatus(x->status),x->command);
    char *str = (char*)malloc(200 * sizeof(char));
    sprintf(str, "#%d, %s: %s\n", x->id,resOfStatus(x->status),x->command);
    return str;
}


void imprimeTExec (Tarefa x){
    //printf("Tarefa #%d: pid - %d, status - %d, command - %s\n",x->id,x->pid,x->status,x->command);
    printf("#%d: %s\n",x->id,x->command);
}

char* stringTExec (Tarefa x){
    char *str = (char*)malloc(200 * sizeof(char));
    sprintf(str, "#%d: %s\n", x->id,x->command);
    return str;
}

void imprimeLExec (TList x){
    while(x){
        imprimeTExec(x->tarefa);
        x=x->next;
    }
}