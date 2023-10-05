#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "parser.h"
#include "time.h"

#define BUFLEN 1024

int execute(char* parsedInput, char** splitInput, int inputCount, bool background);

int main() {
    char buffer[1024];
    char* unparsedInput;
    char* args[3];
    char newline;
    int inputCount;
    bool waitFlag = true;
    int pipeIndex;

    printf("Welcome to the Group03 shell! Enter commands, enter 'quit' to exit or 'help' for a list of commands!\n");
    do {
        bool pipeFlag = false;
        waitFlag = true;
        
        //Print the terminal prompt and get input
        printf("$ ");
        char *input = fgets(buffer, sizeof(buffer), stdin);

        if(!input)
        {
            fprintf(stderr, "Error reading input\n");
            return -1;
        }

        //Clean and parse the input string
        unparsedInput = (char*) malloc(BUFLEN * sizeof(char));
        char* parsedInput = trimstring(unparsedInput, input, BUFLEN);
        
        if (parsedInput == 0) {
            // do nothing
        } else {
            char **splitInput = split(parsedInput, ' ', &inputCount);
            
        // Split input tester
            // for(int i = 0; i < inputCount; i++){
            //     printf("%d: %s\n", i, splitInput[i]);
            // }


            // Background Check
            if(strcmp(splitInput[inputCount - 1],"&") == 0 && splitInput[0]!="echo") {
                waitFlag = false;
                free(splitInput[inputCount - 1]);
                inputCount--;
            }
            
            // Pipe Detection
            for(int i = 0; i < inputCount; i++){
                if(strcmp(splitInput[i], "|") == 0){
                    pipeFlag = true;
                    pipeIndex = i;
                    break;
                }
            }
            
            // Quit
            if(strcmp(splitInput[0], "quit") == 0) {
                printf("Farewell, Comrade...\n");
                return 0;
            }
            
            if(pipeFlag == false) {
                pid_t backgroundPID = waitpid(-1, NULL, WNOHANG);
                if (backgroundPID != 0 && backgroundPID != -1) {
                    printf("\nBackground command %d terminated\n\n", backgroundPID);
                    waitFlag = true;
                }
                execute(parsedInput, splitInput, inputCount, waitFlag);
            } 

            else {
                // Split the command into two parts: before and after the pipe
                int comcount;
                char** commands = split(parsedInput, '|', &comcount);
                strip(commands[1], ' ');

                int com1count;
                int com2count;
                char** command1 = split(commands[0], ' ', &com1count);
                char** command2 = split(commands[1], ' ', &com2count);

                free(command1[com1count - 1]);
                com1count--;


                int pipefd[2];
                if (pipe(pipefd) == -1){
                    perror("Pipe creation failed\n");
                    return -1;
                }

                pid_t forkV1 = fork();
                if (forkV1 == 0){
                    // Child 1: Execute the first command and write to the pipe
                    close(pipefd[0]); // Close the read end of the pipe
                    dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to the write end of the pipe
                    close(pipefd[1]); // Close the write end of the pipe

                    // Execute the first command
                    int ret = execute(command1[0], command1, com1count, true);
                    if (ret == -1) {
                        perror("Command execution failed 1\n");
                    }
                    exit(1);
                }
                else {
                    // Parent
                    close(pipefd[1]); // Close the write end of the pipe

                    pid_t forkV2 = fork();
                    if (forkV2 == 0) {
                        // Child 2:  Read from the pipe and execute the second command
                        dup2(pipefd[0], STDIN_FILENO); // Redirect stdin to the read end of the pipe
                        close(pipefd[0]); // Close the read end of the pipe

                        // Execute the second command
                        int ret = execute(command2[0], command2, com2count, true);
                        if (ret == -1) {
                            perror("Command execution failed 2\n");
                        }
                        exit(1);
                    }
                    else {
                        close(pipefd[0]); // Close the read end of the pipe
                        wait(NULL); // Wait for child 2 to finish
                    }
                    wait(NULL); // Wait for child 1 to finish
                
                }
            }
            //Remember to free any memory you allocate!
            if(parsedInput != NULL) {
                for (int i = 0; i < inputCount; i++) {
                    free(splitInput[i]);
                }
                free(splitInput);
            }
            free(unparsedInput);
        }

    } while ( 1 );

    return 0;
}

/*-----------------------------------------------------------------------------------------------------------*/
int execute(char* parsedInput, char** splitInput, int inputCount, bool waitFlag) {
    char* args[3];
    
    // not absolute path
    if(access(splitInput[0], F_OK) == 0) {
        pid_t forkV = fork();
        if ( forkV == 0 ) { 
            if(execve(splitInput[0], splitInput, NULL))
            {
                fprintf(stderr, "Error running command in execve\n");
                exit(-100);
                return -1;
            }
            return 1; 
        } else {
            // wait(NULL); 
            if (waitFlag == true) { 
                wait(NULL); 
            }
        }
    } 

/*-----------------------------------------------------------------------------------------------------------*/
    // Ayaya command
    if (strcmp(splitInput[0], "ayaya") == 0) {
        int repeat = 1;

        for(int i = 1; i < inputCount; i++) {
            if(strcmp(splitInput[i], "-r") == 0){
                if((i + 1 == inputCount)){
                    repeat = 0;
                    printf("No Argument after -r\n");
                    break;
                }
                if(atoi(splitInput[i + 1]) == 0 ){
                    repeat = 0;
                    printf("Invalid Argument: %s\n", splitInput[i+1]);
                    break; 
                }
                repeat = atoi(splitInput[i+1]);
                printf("repeating %d times...\n", repeat);
                break;
            }
        }
            
        for(int i = 0; i < repeat; i++) {
            pid_t forkV = fork();
            if ( forkV == 0 ) {
                args[0] = "/usr/bin/echo";
                args[1] = "\n⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣬⡛⣿⣿⣿⣯⢻\n"
                            "⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡟⢻⣿⣿⢟⣻⣿⣿⣿⣿⣿⣿⣮⡻⣿⣿⣧\n"
                            "⣿⣿⣿⣿⣿⢻⣿⣿⣿⣿⣿⣿⣆⠻⡫⣢⠿⣿⣿⣿⣿⣿⣿⣿⣷⣜⢻⣿\n"
                            "⣿⣿⡏⣿⣿⣨⣝⠿⣿⣿⣿⣿⣿⢕⠸⣛⣩⣥⣄⣩⢝⣛⡿⠿⣿⣿⣆⢝\n"
                            "⣿⣿⢡⣸⣿⣏⣿⣿⣶⣯⣙⠫⢺⣿⣷⡈⣿⣿⣿⣿⡿⠿⢿⣟⣒⣋⣙⠊\n"
                            "⣿⡏⡿⣛⣍⢿⣮⣿⣿⣿⣿⣿⣿⣿⣶⣶⣶⣶⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿\n"
                            "⣿⢱⣾⣿⣿⣿⣝⡮⡻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠿⠛⣋⣻⣿⣿⣿⣿\n"
                            "⢿⢸⣿⣿⣿⣿⣿⣿⣷⣽⣿⣿⣿⣿⣿⣿⣿⡕⣡⣴⣶⣿⣿⣿⡟⣿⣿⣿\n"
                            "⣦⡸⣿⣿⣿⣿⣿⣿⡛⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇⣿⣿⣿\n"
                            "⢛⠷⡹⣿⠋⣉⣠⣤⣶⣶⣿⣿⣿⣿⣿⣿⡿⠿⢿⣿⣿⣿⣿⣿⣷⢹⣿⣿\n"
                            "⣷⡝⣿⡞⣿⣿⣿⣿⣿⣿⣿⣿⡟⠋⠁⣠⣤⣤⣦⣽⣿⣿⣿⡿⠋⠘⣿⣿\n"
                            "⣿⣿⡹⣿⡼⣿⣿⣿⣿⣿⣿⣿⣧⡰⣿⣿⣿⣿⣿⣹⡿⠟⠉⡀⠄⠄⢿⣿\n"
                            "⣿⣿⣿⣽⣿⣼⣛⠿⠿⣿⣿⣿⣿⣿⣯⣿⠿⢟⣻⡽⢚⣤⡞⠄⠄⠄⢸⣿\n";
                args[2] = NULL;
                if(execve("/usr/bin/echo", args, NULL) == -1)
                {
                    fprintf(stderr, "Error running command in execve\n");
                    exit(-100);
                    return -1;
                }
            
                return 1;

            } else {
                //wait(NULL);
                if (waitFlag == true) { 
                    wait(NULL); 
                }
            } 
        }
    } 
/*-----------------------------------------------------------------------------------------------------------*/
    // echo command
    else if (strcmp(splitInput[0], "echo") == 0){
        pid_t forkV = fork();

        char temp[1024] = "\0";
        
        for(int i = 1; i < inputCount; i++){
            strcat(temp, splitInput[i]);
            strcat(temp, " ");
        }

        if ( forkV == 0 ) {
            args[0] = "/usr/bin/echo";
            args[1] = temp;
            args[2] = NULL;
            if(execve("/usr/bin/echo", args, NULL) == -1)
            {
                fprintf(stderr, "Error running command in execve\n");
                exit(-100);
                return -1;
            }
            return 1;

        } else {
            //wait(NULL); 
            if (waitFlag == true) { 
                wait(NULL); 
            }
        }  
    }
/*-----------------------------------------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------------------------------------*/
    // date command       
    else if (strcmp(splitInput[0], "date") == 0 ) {
        pid_t forkV = fork();
        if ( forkV == 0 ) {
            args[0] = "/usr/bin/date";
            args[1] = NULL;
            args[2] = NULL;
            if(execve("/usr/bin/date", args, NULL) == -1)
            {
                fprintf(stderr, "Error running command in execve\n");
                exit(-100);
                return -1;
            }
            return 1;

        } else {
            //wait(NULL); 
            if (waitFlag == true) { 
                wait(NULL); 
            }
        }  
    }
            
/*-----------------------------------------------------------------------------------------------------------*/
    // help command
    else if ( strcmp(splitInput[0], "help") == 0 ) {
        pid_t forkV = fork();
        if ( forkV == 0 ) {
            args[0] = "/usr/bin/echo";
            args[1] = 
                "\nThese are a few of our shell feature commands...\n"
                "   quit : terminates the shell program\n"
                "   ayaya : if you know, then you know, if not then try it and find out\n"
                "       -r [times] : do it [times] times!\n"
                "   date : tells you the current date and time\n"
                "   ls : shows the current working directory, which is wherever this is!\n"
                "   whatever else : it echoes whatever else you typed!\n"
                ;
            args[2] = NULL;
            if(execve("/usr/bin/echo", args, NULL) == -1)
            {
                fprintf(stderr, "Error running command in execve\n");
                exit(-100);
                return -1;
            }
            return 1;

        } else {
            //wait(NULL); 
            if (waitFlag == true) { 
                wait(NULL); 
            }
        }   
    }  
/*-----------------------------------------------------------------------------------------------------------*/
    // sleep command
    else if ( strcmp(splitInput[0], "sleep") == 0 ) {
        pid_t forkV = fork();
        if ( forkV == 0 ) {
            args[0] = "/usr/bin/sleep";
            args[1] = splitInput[1];
            args[2] = NULL;
            if(execve("/usr/bin/sleep", args, NULL) == -1)
            {
                fprintf(stderr, "Error running command in execve\n");
                exit(-100);
                return -1;
            }
            return 1;

        } else {
            //wait(NULL); 
            if (waitFlag == true) { 
                wait(NULL); 
            }
        }
    }

/*-----------------------------------------------------------------------------------------------------------*/

    // if the command is not built in, check if it's in /usr/bin/{command}
    else if(access(splitInput[0], F_OK) != 0){
        char temp[] = "/usr/bin/";
        char* check = (char*)malloc((strlen(temp) + strlen(splitInput[0]) + 1) * sizeof(char));
        strcpy(check, temp);
        strcat(check, splitInput[0]);

        if (access(check, F_OK) == 0){
            free(splitInput[0]);
            splitInput[0] = check;
            
            pid_t forkV = fork();
            if ( forkV == 0 ) { 
                if(execve(splitInput[0], splitInput, NULL))
                {
                    fprintf(stderr, "Error running command in execve\n");
                    exit(-100);
                    return -1;
                }
                return 1;
            } else {
                //wait(NULL); 
                if (waitFlag == true) { 
                    wait(NULL); 
                }
            } 
        }

        else { // echo if no command
            pid_t forkV = fork();

            if ( forkV == 0 ) {
                args[0] = "/usr/bin/echo";
                args[1] = "No such command...";
                args[2] = NULL;
                if(execve("/usr/bin/echo", args, NULL) == -1)
                {
                    fprintf(stderr, "Error running command in execve\n");
                    exit(-100);
                    return -1;
                }
                return 1;
            } else {
                //wait(NULL); 
                if (waitFlag == true) { 
                    wait(NULL); 
                }
            }
        }
    }
}