#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
//Max input length with space for null terminator
#define MAXIN 2049
//Global variable for checking background
int checkBG = 1;
//Handles signal for SIGINT background to allow/discontinue background proccesses
void signalHandler() {
	if(checkBG == 1) {
    checkBG = 0;
    printf("Entering foreground-only mode (& is now ignored)\n");
	}
	else {
		checkBG = 1;
    printf("Exiting foreground-only mode\n");
	}
  fflush(stdout);
}
//Function to get user input from command line of shell
char* userIn() {
  //set input buffer
  char* input = malloc(MAXIN * sizeof(char));

  printf(": ");
	fflush(stdout);
  //get input
	fgets(input, MAXIN, stdin);
  //remove newline character
  input[strlen(input) - 1] = '\0';

  return input;
}

//Function to go through each character in each argument to find "$$" and preform variable expansion to processID
void varExpan(char * cmd[]) {
  int i;
  int j;
  for(i = 0;cmd[i]; i++) {
    for(j=0; cmd[i][j]; j++) {
      if(cmd[i][j] == '$' && cmd[i][j+1] == '$') {
        cmd[i][j] = '\0';
        sprintf(cmd[i], "%s%d", cmd[i], getpid());
      }
    }
  }
}
//Break up input into command componenets
void cmdLine(char* cmd[], int* bg, char inFile[], char outFile[]) {
  //get input
  char* input = userIn();

  //if input was empty, set command to empty and exit function
	if(strcmp(input, "") == 0) {
		cmd[0][0] = '#';
		return;
	}

  //tokenize string into command components
	const char delim[2] = " ";
	char *arg = strtok(input, delim);
  //keep tokenizing until arg returns NULL
  int i = 0;
	while(arg) {
    //If arg is "<" set inputFile name to next tokenized component
		if(strcmp(arg, "<") == 0) {
			arg = strtok(NULL, delim);
			strcpy(inFile, arg);
		}
    //If arg is ">" set outputFile name to next tokenized component
		else if(strcmp(arg, ">") == 0) {
			arg = strtok(NULL, delim);
			strcpy(outFile, arg);
		}
    //If arg is "&" set bg flag to run proccesses in background
    else if(strcmp(arg, "&") == 0) {
			*bg = 1;
		}
    //set command to to first index of string cmd array
		else {
			cmd[i] = strdup(arg);
      //check for variable expansion to processID
      varExpan(cmd);
		}
    //go to next token
    i++;
		arg = strtok(NULL, delim);
	}
  //free user input
  free(input);
}

//Runs command from from user
void run(char* cmd[], char* inFile, char* outFile, int* exitStat, int* bg, struct sigaction sigHand) {
  int result;
  //spawn child process
	pid_t spawnPid = -5;
	spawnPid = fork();
	switch(spawnPid) {
    //if spawning child fails, send error
		case -1:	{
			perror("Error spawning child!\n");
			exit(1);
			break;
    }
		case 0:	{
      //set signal handle to default for child
			sigHand.sa_handler = SIG_DFL;
			sigaction(SIGINT, &sigHand, NULL);

      //if inputFile string is not empty, open file for reading
			if(strcmp(inFile, "") != 0) {
				int in = open(inFile, O_RDONLY);
        //if file cannot be opened, send error
				if(in == -1) {
					printf("cannot open %s for input\n", inFile);
          fflush(stdout);
					exit(1);
				}
        //redirect stdin to inputFile descriptor
				result = dup2(in, 0);
        //if error of copying file descriptor, send error
				if(result == -1) {
          perror("ERROR: input file redirection\n");
					exit(2);
				}
        //close file descriptor on exec()
				fcntl(in, F_SETFD, FD_CLOEXEC);
			}
      //if outputFile string is not empty, open file for writing
			if(strcmp(outFile, "") != 0) {
				int out = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        //if file cannot be opened, send error
				if(out == -1) {
          printf("cannot open %s for output\n", outFile);
          fflush(stdout);
					exit(1);
				}
        //redirect stdout to outputFile descriptor
				result = dup2(out, 1);
        //if error with redirection, send error
				if(result == -1) {
					perror("ERROR: output file redirection\n");
					exit(2);
				}
        //close file descriptor on exec()
				fcntl(out, F_SETFD, FD_CLOEXEC);
			}
      //execute command with cmd arguments, if it fails print error
			if(execvp(cmd[0], (char* const*)cmd)) {
				printf("%s: no such file or directory\n", cmd[0]);
				fflush(stdout);
				exit(2);
			}
			break;
    }
		default:	{
      //check bg flag and if background processes are allowed
			if(*bg == 1 && checkBG == 1) {
        //don't wait for background process to finish with WNOHANG
				pid_t thisPid = waitpid(spawnPid, exitStat, WNOHANG);
        //print background processID
				printf("background pid is %d\n", spawnPid);
				fflush(stdout);
			}
      //otherwise, wait for this child process (foreground) to finish
			else {
				pid_t thisPid = waitpid(spawnPid, exitStat, 0);
			}
    }
    //catch any child process (background) that terminates
		while((spawnPid = waitpid(-1, exitStat, WNOHANG)) > 0) {
			printf("child %d terminated\n", spawnPid);
      //check how process terminated
      if(WIFEXITED(*exitStat)) {
        //print exit value
    		printf("exit value %d\n", WEXITSTATUS(*exitStat));
    	}
      else {
        //print signal value
    		printf("terminated by signal %d\n", WTERMSIG(*exitStat));
    	}
			fflush(stdout);
		}
	}
}

int main() {
	char inputFile[MAXIN/2];
	char outputFile[MAXIN/2];
	char* userInput[513];
  //empty out variables
  memset(inputFile, '\0', sizeof(inputFile));
  memset(outputFile, '\0', sizeof(outputFile));
  int i;
	for(i=0; i<513; i++) {
		userInput[i] = NULL;
	}

	//sigaction to handle signal SIGINT
	struct sigaction signalINT;
	signalINT.sa_handler = SIG_IGN;
	sigfillset(&signalINT.sa_mask);
	signalINT.sa_flags = 0;

  //sigaction to handle signal SIGTSTP
	struct sigaction signalTSTP;
	signalTSTP.sa_handler = signalHandler;
	sigfillset(&signalTSTP.sa_mask);
	signalTSTP.sa_flags = 0;

  sigaction(SIGINT, &signalINT, NULL);
	sigaction(SIGTSTP, &signalTSTP, NULL);

  //start shell
  int notEnd = 1;
  int exitStatus = 0;
  int bg = 0;
	while(notEnd) {
    //get command line from  user input
		cmdLine(userInput, &bg, inputFile, outputFile);
    //check input for comment or empty, and skip over if true
		if(userInput[0][0] == '#' || userInput[0] == NULL || userInput[0][0] == '\0') {}
    //check for exit command and end loop for shell
		else if(strcmp(userInput[0], "exit") == 0) {
			notEnd = 0;
		}
    //check command for "cd"
		else if(strcmp(userInput[0], "cd") == 0) {
      //check if argument was passed for changing directory
			if(userInput[1]) {
        //change directory, if failed, send error
				if(chdir(userInput[1]) == -1) {
					printf("Cannot open directory: %s\n", userInput[1]);
					fflush(stdout);
				}
			}
      //if no argument, get HOME
      else {
				chdir(getenv("HOME"));
			}
		}
    //check command for "status"
		else if(strcmp(userInput[0], "status") == 0) {
      //check what caused last process termination
      if(WIFEXITED(exitStatus)) {
        //display exit value
    		printf("exit value %d\n", WEXITSTATUS(exitStatus));
    	}
      else {
        //display signal value
    		printf("terminated by signal %d\n", WTERMSIG(exitStatus));
    	}
		}
    //otherwise, run command given from user
		else {
			run(userInput, inputFile, outputFile, &exitStatus, &bg, signalINT);
		}
    //reset variables for next command line prompt
		for(i=0; userInput[i]; i++) {
      free(userInput[i]);
			userInput[i] = NULL;
		}
		bg = 0;
		inputFile[0] = '\0';
		outputFile[0] = '\0';
	}
	return 0;
}
