#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>

#define DELIMITERS " \n"
#define DEL2 "|"
#define BUFFER_MIN_SIZE 128
#define MAX_HISTORY 15

int interactive = 0;
char * history[MAX_HISTORY];
int history_end=0;







int start_child(char **argument_buffer)
{
	pid_t pid, wpid;
	int running_status;

	pid = fork();
	if (pid == 0) {

		if (execvp(argument_buffer[0], argument_buffer) == -1) {
			printf("Invalid command \n");
		}
		exit(-1);
	} else if (pid < 0) {

		printf("error forking \n");
	} else {

		waitpid(pid, &running_status, WUNTRACED);
		while (WIFSIGNALED(running_status) == 0
		       && WIFEXITED(running_status) == 0) {
			waitpid(pid, &running_status, WUNTRACED);
		}

	}

	return 1;
}

void piper(char * args[])
{
	int fd1[2];
	int fd2[2];
	int num_pipes=0;
	int num_commands =0;
	int i=0,j=0,k=0,l=0,exit=0,pipe_number=0;
	pid_t pid;
	char * command[128];

	while(args[i]!=NULL)
	{
		//printf("%s \n",args[i]);fflush(stdout);
		if(strcmp(args[i],"|")==0)
		{
			num_pipes++;
		}
		i++;
	}
	i=0;

	while(exit==0 && args[i]!=NULL )
	{
		num_commands=0;
		while( args[i]!=NULL && strcmp(args[i],"|")!=0)
		{
			command[num_commands]=args[i];
			num_commands++;
			i++;
			if(args[i]==NULL)exit=1;
		}
		command[num_commands]=NULL;
		i++;//skip the pipe
		// printf("commands\n");fflush(stdout);
		// 	 while(l<num_commands){
		// 	 	printf("%s \n",command[l]);fflush(stdout);
		// 	 	l++;
		// 	 }

		if(pipe_number%2!=0)
		{
			pipe(fd1);
		}
		else
		{
			pipe(fd2);
		}
		pid = fork();
		if(pid==-1)
		{
			if(pipe_number!=num_pipes)
			{
				if(pipe_number%2==0)
				{
					close(fd2[1]);
				}
				else
				{
					close(fd1[1]);
				}
			}
			printf("error\n");
			return;
		}
		if(pid==0)
		{
			//printf("Got into child\n");fflush(stdout);
			if(pipe_number==0)
			{
				//printf("pipe 0\n");
				dup2(fd2[1],STDOUT_FILENO);
			}
			else if(pipe_number == num_pipes)
			{
				if(num_pipes%2==0)
				{
					dup2(fd1[0],STDIN_FILENO);
				}
				else
				{
					dup2(fd2[0],STDIN_FILENO);
				}
			}
			else
			{
				if(pipe_number%2!=0)
				{
					dup2(fd2[0],STDIN_FILENO);
					dup2(fd1[1],STDOUT_FILENO);
				}
				else
				{
					dup2(fd1[0],STDIN_FILENO);
					dup2(fd2[1],STDOUT_FILENO);
				}
			}
			if(execvp(command[0],command)==-1)
			{
				kill(getpid(),SIGTERM);
			}
			
		}
		if(pipe_number==0)
		{
			close(fd2[1]);
		}
		else if(pipe_number==num_pipes)
		{
			if(num_pipes%2==0)
			{
				close(fd1[0]);
			}
			else
			{
				close(fd2[0]);
			}
		}
		else
		{
			if(pipe_number%2!=0)
			{
				close(fd2[0]);
				close(fd1[1]);
			}
			else
			{
				close(fd1[0]);
				close(fd2[1]);
			}
		}
	waitpid(pid,NULL,0);
		pipe_number++;
	}
}

void IOredirect(char * args[])
{
	int i=0,j=0,k=0,l=0,exit=0;
	while(exit ==0 && args[i]!=NULL)
	{

		int num_redirects =0;
		while(args[l]!=NULL)
		{
			if(strcmp(args[l],">")==0 || strcmp(args[l],"<")==0)
			{
				num_redirects++;
			}
			l++;
		}
		char * command[128];
		int command_end=0;
		while(args[i]!=NULL && args[i]!="<" && args[i]!=">" )
		{		
			command[command_end] = args[i];
			command_end++;
			i++;
			if(args[i]==NULL)
				{
					exit=1;	
				}
		}
		//i++;//skip < or >
		command[command_end]=NULL;


	}
}

void singleIOredirect(char * args[])
{
		int i=0,j=0,k=0,l=0,exit=0;
		
		char * command[128];
		int command_end=0;
		while(args[i]!=NULL && strcmp(args[i],"<")!=0 && strcmp(args[i],">")!=0 )
		{		
			i++;
		}
		if(strcmp(args[i],">")==0)
		{
			j=0;
			while(j<i)
			{
				command[command_end]=args[j];
				j++;
				command_end++;
			}
			command[command_end]=NULL;
			if(args[i+1]==NULL)
			{
				printf("Wrong argument\n");
				return;
			}
			else
			{
//				int in = open(args[i+1],O_RDONLY);
//				dup2(in,STDIN_FILENO);
//				close(in);
				pid_t p1 = fork();
				if(p1==0){
			 	int out = open(args[i+1],O_WRONLY|O_CREAT,0666); // Should also be symbolic values for access rights
				dup2(out,STDOUT_FILENO);
				close(out);
				execvp(command[0],command);
			}
			else
			{
				wait(&p1);
			}
			}
		}
		else if(strcmp(args[i],"<")==0)
		{
			j=0;
			while(j<i)
			{
				command[command_end]= args[j];
				j++;
				command_end++;
			}
			command[command_end]=NULL;
			pid_t p2 = fork();
			if(p2==0){
			int in = open(args[i+1],O_RDONLY);
			if(in == -1)
			{
				printf("Wrong arguments\n");
				return 0;
			}
			dup2(in,STDIN_FILENO);
			close(in);
			 // 	int out = open("output",O_WRONLY|O_CREAT,0666); // Should also be symbolic values for access rights
				// dup2(out,STDOUT_FILENO);
				// close(out);
				execvp(command[0],command);}
				else
				{
					wait(&p2);
				}
		}

}

void sigintHandler(int sig_num)
{
    signal(SIGINT, sigintHandler);
    printf("\nush> ");
    fflush(stdout);
}


int main()
{
	int running_status = 1;
	while (running_status > 0) {
		printf("ush> ");
		char *line = NULL;
		ssize_t bufsize = 0;	// have getline allocate a buffer for us
		signal(SIGINT, sigintHandler);
		getline(&line, &bufsize, stdin);
		//strcpy(history[history_end%MAX_HISTORY], line);history_end++;history_end%=MAX_HISTORY;
		//history[history_end%MAX_HISTORY]=line;history_end++;history_end%=MAX_HISTORY;
		history[history_end]=malloc(sizeof(char)*bufsize);
		strcpy(history[history_end%MAX_HISTORY],line);
		history_end++;history_end%=MAX_HISTORY;
		
		int reallocation_int = 1;
		
		char **argument_buffer = malloc(sizeof(char *) * reallocation_int * BUFFER_MIN_SIZE);
		int argument_buffer_size = 0;

		char *token = NULL;
		token = strtok(line, DELIMITERS);
		while (token != NULL) {
				argument_buffer[argument_buffer_size] = token;
				argument_buffer_size++;
			if (argument_buffer_size + 1 >=BUFFER_MIN_SIZE * reallocation_int) {
				reallocation_int++;
				argument_buffer =
				    realloc(argument_buffer,
					    reallocation_int * BUFFER_MIN_SIZE *
					    sizeof(char *));
				if (argument_buffer == NULL) {
					printf
					    ("Memory full / allocation error\n");
					exit(-1);
				}
			}
			token = strtok(NULL, DELIMITERS);
		}
		argument_buffer[argument_buffer_size] = NULL;
		int i=0;
		//piper(argument_buffer);
		int meh=1;
		while(i<argument_buffer_size)
		{
			if(strcmp(argument_buffer[i],"|")==0)
			{
				meh=0;
				piper(argument_buffer);
				break;
			}
			else if(strcmp(argument_buffer[i],"<")==0 || strcmp(argument_buffer[i],">")==0   )
			{
				singleIOredirect(argument_buffer);
				meh=0;
				break;
			}
			i++;
		}
		if(meh)
		{	
			 if (argument_buffer_size >= 1) {
			 	char *special[5] = { "cd", "exit" ,"history","help"};
			 	int i = 0, flag = 0;	
			 	if (strcmp("exit", argument_buffer[0]) == 0) {
			 		flag = 1;
					return 0;

				} 
				else if(strcmp("history",argument_buffer[0])==0)
			 	{
			 		flag=1;
			 		int temp=0;
			 		while(temp<history_end)
			 		{
			 			printf("%s\n",history[temp]);
			 			temp++;
			 		}
			 	}
				else if (strcmp("cd", argument_buffer[0]) == 0) {
			 		flag = 1;
			 		if (argument_buffer_size <= 1) {
			 			printf("Wrong arguments\n");
					} else {
						chdir(argument_buffer[1]);
			 			}
			 	}
			 	else if(strcmp("help",argument_buffer[0])==0)
			 	{
			 		flag=1;
			 		FILE * fptr = fopen("manual","r");
			 		if(fptr==NULL)
			 		{
			 			printf("Couldn't open manual\n");
			 		}else{
			 		char c;
			 		c = fgetc(fptr);
			   	 	while (c != EOF)
			    	{
			    	    printf ("%c", c);
			    	    c = fgetc(fptr);
			    	}
 						
 					fclose(fptr);
 				}
			 	}

			 	if (flag == 0) {
			 		flag = 1;
			 		running_status = start_child(argument_buffer);
			 	}
			 }
		}






		// if (argument_buffer_size >= 1) {
		// 	char *special[3] = { "cd", "exit" };
		// 	int i = 0, flag = 0;	
		// 	if (strcmp("exit", argument_buffer[0]) == 0) {
		// 		flag = 1;
		// 		return 0;

		// 	} else if (strcmp("cd", argument_buffer[0]) == 0) {
		// 		flag = 1;
		// 		if (argument_buffer_size <= 1) {
		// 			printf("Wrong arguments\n");
		// 		} else {
		// 			chdir(argument_buffer[1]);
		// 		}
		// 	}

		// 	if (flag == 0) {
		// 		flag = 1;
		// 		running_status = start_child(argument_buffer);
		// 	}
		// }
		free(argument_buffer);
		free(line);

	}
}
