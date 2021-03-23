#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#include "collection.h"


int tempo_inatividade;
int tempo_execucao;
int next_id;
TList* em_execucao;
TList* historico;

int in;
int out;
int history_file;
int log_file;
int logidx_file;


void format_arg(char* arg) {
	//if(arg[0] == '\'' || arg[0] == '"' || arg[0] == '´`' ) {
		printf("antes: %s\n",arg);
		int i,len=strlen(arg);
		for(i=1;i<len-1;i++)
		{
			arg[i-1]=arg[i];
		}
		arg[i-1]='\0';
		printf("depois: %s\n",arg);
	//}
}

void nova_tarefa(char* arg) {
	char str1[32];
	char str2[32];
	sprintf(str1, "%d", tempo_execucao);
	sprintf(str2, "%d", tempo_inatividade) ;
	format_arg(arg);


	char* exec_args[] = {"./run",arg,str1,str2,NULL};
	for(int i = 0; i < 4; i++) {
		printf("Exec: arg nº%d: %s\n",i,exec_args[i] );
	}
	int pid = fork();
	switch(pid) {
		case -1:
			perror("fork");
		case 0:
			execv("./run",exec_args);
			_exit(0); //FIXME - talvez este valor não seja o mais correto
	}

	char* str = malloc(200 * sizeof(char));
	sprintf(str, "nova tarefa #%d\n", next_id);
	write(out,str,strlen(str));
	write(out,"",1);
	free(str);


	addTarefa(em_execucao,initTarefa(next_id++,pid,0,arg));
}


void read_history_file() {
	lseek(history_file, 0, SEEK_SET);
	Tarefa ret = malloc(sizeof(struct tarefa));
	printf("A ler do historico\n");
	while(read(history_file,ret,sizeof(struct tarefa)) > 0) {
		char* str = stringTStatus(ret);
		write(out,str,strlen(str));
		imprimeTStatus(ret);
	}
	write(out,"\0",2);
	free(ret);	
	lseek(history_file, 0, SEEK_END);
}

void kill_em_execucao(int pid) {
	printf("Pedido para matar %d\n", pid);
	kill(pid,SIGUSR2);
}

void ler_output(int id) {
	int encontrado = 0;
	lseek(logidx_file, 0, SEEK_SET);
	Index index;
	while(!encontrado  && (read(logidx_file,&index,sizeof(struct index_entry)) > 0)) {
		if(index.id == id) encontrado = 1;
	}
	int fim = index.last_pos;
	//lseek(logidx_file, -2 * sizeof(struct index_entry), SEEK_CUR);

	lseek(logidx_file, -1 * sizeof(struct index_entry), SEEK_CUR);
	int inicio;

	if(lseek(logidx_file, 0, SEEK_CUR) != 0) {
		lseek(logidx_file, -1 * sizeof(struct index_entry), SEEK_CUR);
		read(logidx_file,&index,sizeof(struct index_entry));
		inicio = index.last_pos;
	} else {
		inicio = 0;
	}

	char* buf = malloc((fim-inicio+1) * sizeof(char));

	if(inicio != 0) {
		lseek(log_file, inicio, SEEK_SET);
	} else {
		lseek(log_file, 0, SEEK_SET);
	}
	read(log_file,buf,fim-inicio);
	buf[fim-inicio] = '\0';
	printf("log: %s\n", buf);
	write(out,buf,fim-inicio+1);
	lseek(log_file, 0, SEEK_END);
	lseek(logidx_file, 0, SEEK_END);
	write(out,"",1);

	free(buf);

}

void print_help() {
	char str[100] = "tempo-inactividade segs\n";
	write(out,str,strlen(str));
	strcpy(str,"tempo-execucao segs\n");
	write(out,str,strlen(str));
	strcpy(str,"executar p1 | p2 ... | pn\n");
	write(out,str,strlen(str));
	strcpy(str,"listar\n");
	write(out,str,strlen(str));
	strcpy(str,"terminar  id\n");
	write(out,str,strlen(str));
	strcpy(str,"historico\n");
	write(out,str,strlen(str));
	strcpy(str,"output id\n");
	write(out,str,strlen(str));
	strcpy(str,"ajuda\n");
	write(out,str,strlen(str));
	write(out,"",1);
}


void print_em_execucao() {
	TList x = *em_execucao;
	char* str;
    while(x){
    	str = malloc(200 * sizeof(char));
    	sprintf(str, "#%d: %s\n", x->tarefa->id,x->tarefa->command);
		write(out,str,strlen(str));
		free(str);
        x=x->next;
    }
    write(out,"\n",1);
	write(out,"",1);

}

void exec_command(char* opcode, char* arg) {
	if (strcmp(opcode, "tempo-inactividade") == 0) {
		tempo_inatividade = atoi(arg);
		printf("Tempo de Inatividade: %d\n", tempo_inatividade);
	} 
	else if (strcmp(opcode, "tempo-execucao") == 0) {
		tempo_execucao = atoi(arg);
		printf("Tempo de Execucao: %d\n", tempo_execucao);
	}
	else if (strcmp(opcode, "executar") == 0) {
		nova_tarefa(arg);
	}
	else if (strcmp(opcode, "historico\n")  == 0 || strcmp(opcode, "historico")  == 0) {
		read_history_file();
	}
	else if (strcmp(opcode, "listar\n")  == 0 || strcmp(opcode, "listar")  == 0) {
		print_em_execucao();
	}
	else if (strcmp(opcode, "terminar\n")  == 0 || strcmp(opcode, "terminar")  == 0) {
		kill_em_execucao(pid_of_id(em_execucao,atoi(arg)));
	}
	else if (strcmp(opcode, "ajuda\n")  == 0 || strcmp(opcode, "ajuda")  == 0) {
		print_help();
	}
	else if (strcmp(opcode, "output")  == 0) {
		ler_output(atoi(arg));
	}
	else /* default: */ {
		printf("nope\n");
	}
}


void sigchild_handler(int sig) {
	int status;
	pid_t pid = waitpid(-1,&status,WNOHANG);
	if(pid > 0) {
		if(!WIFEXITED(status)) {
			printf("Pai (MAIN): recebi o valor %d para o pid %d (interrompido)\n", WEXITSTATUS(status), pid);
		} else {
			printf("Pai (MAIN): recebi o valor %d para o pid %d (acabou corretamente)\n", WEXITSTATUS(status), pid);
		}
		Tarefa ret = remove_by_pid(em_execucao,pid);
		ret->status = WEXITSTATUS(status);

		if(ret != NULL) {
			printf("A escrever no history\n");
			lseek(history_file, 0, SEEK_END);
			write(history_file,ret,sizeof(struct tarefa));

			lseek(log_file, 0, SEEK_END);
			int position = lseek(log_file, 0, SEEK_CUR);
			printf("escreveu até %d\n", position);

			Index index;
			index.id = ret->id;
			index.last_pos = position;
			lseek(logidx_file, 0, SEEK_END);
			write(logidx_file,&index,sizeof(struct index_entry));
			free(ret);
		}

	}
}

int calc_next_id() {
	int ret = -1;
	lseek(logidx_file, 0, SEEK_SET);
	Index index;
	while(read(logidx_file,&index,sizeof(struct index_entry)) > 0) {
		if(index.id > ret) ret = index.id;
	}
	int fim = index.last_pos;
	lseek(logidx_file, 0, SEEK_END);
	return ret+1;
}


void print(char* s) {
	for(int i = 0; i<sizeof(s); i++) {
		printf("%d\n",s[i]);
	}
}


int main(int  argc, char** argv) {
	if(signal(SIGCHLD, sigchild_handler) < 0) {
		perror("signal SIGCHLD");
		exit(-1);
	}

	//FIXME
	history_file = open("./history", O_CREAT | O_RDWR | O_APPEND, 0666);
	log_file = open("./log", O_CREAT | O_RDWR | O_APPEND, 0666);
	logidx_file = open("./log.idx", O_CREAT | O_RDWR | O_APPEND, 0666);


	tempo_inatividade = tempo_execucao = -1;
	next_id = calc_next_id();
	printf("next id: %d\n", next_id);

	em_execucao = initialNode();
	historico = initialNode();

	if(argc < 2) {
	    mkfifo("toServer", 0666);
	    mkfifo("toClient", 0666);

	    if((in = open("toServer" , O_RDONLY)) < 0) {
	    	perror("open fifo");
	    	exit(1);
	    }
	    if((out = open("toClient" , O_WRONLY)) < 0) {
	    	perror("open fifo");
	    	exit(1);
	    }

	    char buf[200];
	    memset(buf, 0, 200);
	    int bytes_read = 0;

	    //while((bytes_read = read(in, buf, 200)) > 0) {
	    while(1) {
	    	while((bytes_read = read(in, buf, 200)) <= 0);

	        char* op;
	        char* arg;
	        char *ptr = strtok(buf, " ");
	        op = strdup(ptr);
			if(ptr = strtok(NULL, "\n")) {
				arg = strdup(ptr);
			} else {
				arg = strdup("");
			}
			printf("opcode: %s - arg: %s\n", op,arg);
			//print(op);

			/*
			if(strcmp(op,"-r\n") == 0) {
				printf("prim\n");
			} else if(strcmp(op,"-r") == 0) {
				printf("secd\n");
			} else {
				printf("nope\n");
			}*/

			exec_command(op,arg);
			memset(buf, 0, 200);	

	    }

	    close(in);
	    close(out);
	    unlink("toClient");
	    unlink("toServer");
	} 

    return 0;
}		