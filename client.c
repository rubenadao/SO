#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#include "argus.h"

int in;
int out;


int main(int argc, char* argv[]) {

	
	if((out = open(TO_SERVER_PIPE,O_WRONLY)) < 0 ) {
		perror("open fifo");
		exit(1);
	}

	if((in = open(TO_CLIENT_PIPE,O_RDONLY)) < 0 ) {
		perror("open fifo");
		exit(1);
	}


	if(argc <= 1) {

		int pid1 = -1;
		int pid2 = -1;

		if((pid1 = fork()) == 0) {
			dup2(out,1);
			close(out);

			execlp("cat","cat",NULL);
			_exit(1);
		}

		if((pid2 = fork()) == 0) {
			dup2(in,0);
			close(in);
			execlp("cat","cat",NULL);
			_exit(1);
		}

		int status = 0;
		wait(&status);
		wait(&status);
	} else {
		int error = 0;
		int toRead = 0;
		if(argc == 2) {
			if(strcmp(argv[1],"-l") == 0) {
				write(out,"listar\n",8);
				toRead = 1;
		 	} else if(strcmp(argv[1],"-r") == 0) {
				write(out,"historico\n",11);
				toRead = 1;
			} else if(strcmp(argv[1],"-h") == 0) {
				write(out,"ajuda\n",7);
				toRead = 1;
			} else error = 1;
		} else if (argc == 3) {
			char* str = malloc(MAXINPUT * sizeof(char));
			if(strcmp(argv[1],"-i") == 0) {
				sprintf(str, "%s %s\n", "tempo-inactividade",argv[2]);
				write(out,str,strlen(str));
			} else if(strcmp(argv[1],"-m") == 0) {
				sprintf(str, "%s %s\n", "tempo-execucao",argv[2]);
				write(out,str,strlen(str));
			} else if(strcmp(argv[1],"-e") == 0) {
				sprintf(str, "%s '%s'\n", "executar",argv[2]);
				write(out,str,strlen(str));
				toRead = 1;
			} else if(strcmp(argv[1],".t") == 0) {
				sprintf(str, "%s %s\n", "terminar",argv[2]);
				write(out,str,strlen(str));
			} else if(strcmp(argv[1],"-o") == 0) {
				sprintf(str, "%s %s\n", "output",argv[2]);
				write(out,str,strlen(str));
				toRead = 1;
			} else error = 1;
		} else {
			error = 1;
		}
		close(out);
		if(toRead) {
			if(fork() == 0) {
				char buf[1];
				int read_byte;
				while((read_byte = read(in, buf, 1)) < 0) {
				}
				//FIXME
				if(read_byte>=1){
					write(1,buf,1);
				}
	   		 	while((read(in, buf, 1)) > 0) {
	   		 		write(1,buf,1);
	   		 		if(buf[0] == '\0'){
	   		 			close(in);
	   		 			close(1);
	   		 			_exit(0);
	   		 		}	
				}
				close(in);
				close(1);
				_exit(1);
			}	
			int status;
			wait(&status);
		}

		if(error) {
			printf("Opção desconhecida\n");
			printf("Use a opção -h para ajuda\n");
		}
	}


	close(out);
	close(in);


	return 0;

}
