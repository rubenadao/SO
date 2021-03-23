#ifndef UNTITLED_TEST_H
#define UNTITLED_TEST_H

typedef struct tarefa {
    int id;
    int pid;
    int status;
    char command[100];
} *Tarefa;

typedef struct index_entry {
    int id;
    int last_pos;
} Index;

typedef struct linkedlist {
    Tarefa tarefa;
    struct linkedlist* next;
} *TList;


Tarefa initTarefa(int id, int pid, int status, char* command);
TList* initialNode();
void addTarefa(TList* list, Tarefa tarefa);
int pop(TList* head);
int remove_by_index(TList* head, int n);
int remove_by_id(TList *l, int id);
Tarefa remove_by_pid(TList *l, int pid);
int pid_of_id(TList *l, int id);
void imprimeL (TList x);
void imprimeT (Tarefa x);
void imprimeTStatus (Tarefa x);
void imprimeTExec (Tarefa x);
void imprimeLExec (TList x);
char* stringTExec (Tarefa x);
char* stringTStatus (Tarefa x);




#endif //UNTITLED_TEST_H