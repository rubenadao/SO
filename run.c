#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#define MAX_LINE_SIZE 1024
#define MAX_COMMANDS 10

//parse do argumento e execvp
//devolve o resultado do exec

int* pids;
int npids;
int log_file;
int final_ret;

int exec_command(char* command) {
	char *exec_args[20];
	char *string;
	int exec_ret = 0;
	int i = 0;

	string = strtok(command, " ");

	while(string != NULL) {
		exec_args[i] = string;
		string = strtok(NULL," ");
		i++;
	}

	exec_args[i] = NULL;

	exec_ret = execvp(exec_args[0],exec_args);

	return exec_ret;
}

void killAll(int ret) {
	for(int i = 0; i<npids; i++) { //FIXME - mata todos até os que não acabaram
		if(pids[i]>0) {
			kill(pids[i],SIGKILL);
		}
	}
	final_ret = ret;
}


void timeout_handler(int sig) {
	//printf("vou matar (tempo)\n");
	killAll(1);
}

void sigusr1_handler(int sig) {
	//printf("vou matar (pipes)\n");
	killAll(2);
}

void sigusr2_handler(int sig) {
	killAll(3);
}


int main(int argc, char** argv) {
	char buf[MAX_LINE_SIZE];
	char* commands[MAX_COMMANDS];
	char* command;
	char* line;
	int cur_pid_pos = 0;
	int pipes[MAX_COMMANDS-1][2];
	int number_of_commands;
	int status[MAX_COMMANDS];
	int res; //para quê??
	int checksTimeout = 0;
	int checksTimePipes = 0;
	int timeout_secs;
	int check_pipe_secs;
	int my_pid = getpid();
	final_ret = 0;



	log_file = open("./log", O_CREAT | O_WRONLY | O_APPEND, 0666);

	dup2(log_file,1);
	close(log_file);



	if(atoi(argv[2]) >= 0) {
		timeout_secs = atoi(argv[2]);
		checksTimeout = 1;
	}

	if(atoi(argv[3]) >= 0) {
		check_pipe_secs = atoi(argv[3]);
		checksTimePipes = 1;
		printf("pipes: %d\n", check_pipe_secs);
	}
	
	if(signal(SIGALRM, timeout_handler) < 0) {
		perror("signal SIGALARM");
		exit(-1);
	}
	if(signal(SIGUSR1, sigusr1_handler) < 0) {
		perror("signal SIGUSR1");
		exit(-1);
	}
	if(signal(SIGUSR2, sigusr2_handler) < 0) {
		perror("signal SIGUSR2");
		exit(-1);
	}


	if( argc > 1) {
		//parse commands
		line = strdup(argv[1]);

		number_of_commands = 0;
		for(number_of_commands = 0; (command=strsep(&line,"|")) != NULL; number_of_commands++) {
			if(number_of_commands != 0) {
				if(checksTimePipes) {
					char str[32];
					sprintf(str, "./checkpipe %d %d", check_pipe_secs, my_pid);
					commands[number_of_commands++] = strdup(str);
				}
			}
			commands[number_of_commands] = strdup(command);
		}

		npids = number_of_commands;
		//printf("n: %d\n", number_of_commands);
		pids = malloc(npids * sizeof(int));

		//if number of commands == 0??
		if(number_of_commands == 1) {
			pids[cur_pid_pos] = fork(); 
			switch(pids[cur_pid_pos]) {
				case -1:
					perror("fork");
					return -1;
				case 0:
					exec_command(commands[0]);
					_exit(0); //FIXME - talvez este valor não seja o mais correto
			}
		} else {
			for(int c = 0; c < number_of_commands; c++) {
				if(c == 0) {
					if(pipe(pipes[c]) != 0) {
						perror("pipe");
						return -1;
					}
					pids[cur_pid_pos] = fork(); 
					switch(pids[cur_pid_pos]) {
						case -1:
							perror("fork");
							return -1;
						case 0:
							close(pipes[c][0]);

							dup2(pipes[c][1],1);
							close(pipes[c][1]);

							exec_command(commands[c]);
							_exit(0);
						default:
							cur_pid_pos++;
							res = close(pipes[c][1]);
					}

				} else if(c == number_of_commands -1) {
					pids[cur_pid_pos] = fork();
					switch (pids[cur_pid_pos]) {
						case -1:
							perror("fork");
							return -1;
						case 0:
							dup2(pipes[c-1][0],0);
							close(pipes[c-1][0]);

							exec_command(commands[c]);
							_exit(0);
						default:
							cur_pid_pos++;
							close(pipes[c-1][0]);
					}
				} else {
					
					if(pipe(pipes[c]) !=0 ) {
						perror("pipe");
						return -1;
					}
					pids[cur_pid_pos] = fork(); 
					switch (pids[cur_pid_pos]) {
						case -1:
							perror("fork");
							return -1;
						case 0:
							close(pipes[c][0]);

							dup2(pipes[c][1],1);
							close(pipes[c][1]);

							dup2(pipes[c-1][0],0);
							close(pipes[c-1][0]);

							exec_command(commands[c]);
							_exit(0);
						default:
							cur_pid_pos++;
							close(pipes[c][1]);
							close(pipes[c-1][0]);
					}
				}

			}
		}

		if(checksTimeout) alarm(timeout_secs);

		/*
		for(int w = 0; w < number_of_commands; w++) {
			wait(&status[w]);
		}
		*/
		for(int w = 0; w < number_of_commands; w++) {
			pid_t filho = wait(&status[w]);
		}

		for(int i = 0; i < number_of_commands; i++) {
			free(commands[i]);
		}

		free(line);

	}
	
	return final_ret; 
}