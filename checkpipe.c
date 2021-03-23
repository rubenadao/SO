#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>


int pid;

void timeout_pipes() {
	//printf("vou matar\n");
	kill(pid,SIGUSR1);
}

int main(int argc, char** argv){
	//printf("estou a correr\n");
	int n;
	char c;

	int timeout;
	if(argc > 1) {
		timeout = atoi(argv[1]);
		if(signal(SIGALRM, timeout_pipes) < 0) {
			perror("signal SIGALARM");
			exit(-1);
		}
		pid = atoi(argv[2]);
	} 

	if(argc > 1)alarm(timeout);
	while( (n = read(0,&c,1)) > 0 ) {
		if(argc > 1) {
			alarm(timeout);
		}
		write(1,&c,1);
	}
	
	return 0;
}
