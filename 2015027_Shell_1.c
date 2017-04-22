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



int main()
{

	char * a[] = {"make","2015027_Shell_2",NULL};
	char * b[] = {"gnome-terminal","-e","./2015027_Shell_2",NULL};
	pid_t xp= fork();
	if(xp !=0 )
	{
		wait(&xp);
	}
	else
	{
		pid_t xp2 = fork();
		if(xp2==0)
		{
			execvp(a[0],a);
			exit(0);
		}
		else
		{
			wait(&xp2);
			execvp(b[0],b);
			exit(0);
		}
	}
	return 0;
}