#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "parser.h"
#include "time.h"

#define BUFLEN 1024

//To Do: This base file has been provided to help you start the lab, you'll need to heavily modify it to implement all of the features

int main() {
    char buffer[1024];
    char* unparsedInput;
    char* args[3];
    char newline;

    int inputCount;

    
    printf("Welcome to the Group03 shell! Enter commands, enter 'quit' to exit\n");
    do {
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
            
            //Split input tester

            // for(int i = 0; i < inputCount; i++){
            //     printf("%d: %s\n", i, splitInput[i]);
            // }

            // not absolute path
            if(access(splitInput[0], F_OK) == 0){
                pid_t forkV = fork();
                char *path;
                char * const envp[] = {path, NULL};
                if ( forkV == 0 ) {
                    path = NULL;
                    if (splitInput[0][0] != '/') {
                        path = getenv("PATH");
                        printf("Executing in: %s\n", envp);
                    }
                    
                    if(execve(splitInput[0], splitInput, envp))
                    {
                        fprintf(stderr, "Error running command in execve\n");
                        return -100;
                    } 
                } else 
                    wait(NULL); 
            }
            
            // Check if /usr/bin/{input} exists
            
            //Sample shell logic implementation
            else if (strcmp(splitInput[0], "quit") == 0 ) {
                printf("Bye!!\n");
                return 0;
            }

/*-----------------------------------------------------------------------------------------------------------*/
            // Ayaya command
            else if ( strcmp(splitInput[0], "ayaya") == 0 ) {
                int repeat = 1;

                for(int i = 1; i < inputCount; i++){
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
                            return -100;
                        }
                    } else 
                        wait(NULL); 
                }
            } 

/*-----------------------------------------------------------------------------------------------------------*/
            // ls
            else if (strcmp(splitInput[0], "ls") == 0 ) {
                pid_t forkV = fork();
                if ( forkV == 0 ) {
                    args[0] = "/usr/bin/ls";
                    args[1] = NULL;
                    args[2] = NULL;
                    if(execve("/usr/bin/ls", args, NULL) == -1)
                    {
                        fprintf(stderr, "Error running command in execve\n");
                        return -100;
                    }

                } else
                    wait(NULL);
            }
/*-----------------------------------------------------------------------------------------------------------*/
            // date command       
            else if ( strcmp(splitInput[0], "date") == 0 ) {
                pid_t forkV = fork();
                if ( forkV == 0 ) {
                    args[0] = "/usr/bin/date";
                    args[1] = NULL;
                    args[2] = NULL;
                    if(execve("/usr/bin/date", args, NULL) == -1)
                    {
                        fprintf(stderr, "Error running command in execve\n");
                        return -100;
                    }

                } else
                    wait(NULL);   
            }

/*-----------------------------------------------------------------------------------------------------------*/
            // help command
            else if ( strcmp(splitInput[0], "help") == 0 ) {
                pid_t forkV = fork();
                if ( forkV == 0 ) {
                    args[0] = "/usr/bin/echo";
                    args[1] = 
                        "These are a few of our shell feature commands...\n"
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
                        return -100;
                    }

                } else
                    wait(NULL);   
            }


/*-----------------------------------------------------------------------------------------------------------*/
            else { // echo if no command
                pid_t forkV = fork();
                if ( forkV == 0 ) {
                    args[0] = "/usr/bin/echo";
                    args[1] = parsedInput;
                    args[2] = NULL;
                    if(execve("/usr/bin/echo", args, NULL) == -1)
                    {
                        fprintf(stderr, "Error running command in execve\n");
                        return -100;
                    }

                } else
                    wait(NULL);
            }

            //Remember to free any memory you allocate!
            if(parsedInput != NULL){
                for (int i = 0; i < inputCount; i++) {
                    free(splitInput[i]);
                }
                free(splitInput);
            }
        }
        free(unparsedInput);

    } while ( 1 );

    return 0;
}
