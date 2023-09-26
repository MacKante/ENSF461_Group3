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
    char* parsedinput;
    char* parsedcommand;
    char* args[3];
    char newline;

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
        parsedinput = (char*) malloc(BUFLEN * sizeof(char));
        size_t parselength = trimstring(parsedinput, input, BUFLEN);

        parsedcommand = (char*) malloc(BUFLEN * sizeof(char));
        size_t commandlength = firstword(parsedinput, input, BUFLEN);

        //Sample shell logic implementation
        if ( strcmp(parsedinput, "quit") == 0 ) {
            printf("Bye!!\n");
            return 0;
        }
        // Ayaya command
        else if ( strcmp(parsedinput, "ayaya") == 0 ) {
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
        // clear command
        else if ( strcmp(parsedinput, "clear") == 0 ) {
            pid_t forkV = fork();
            if ( forkV == 0 ) {
                args[0] = "/usr/bin/";
                args[1] = NULL;
                args[2] = NULL;
                char *envp[] =
                {
                    "HOME=/",
                    "PATH=/bin:/usr/bin",
                    "TZ=UTC0",
                    "USER=mackante",
                    "LOGNAME=mackante",
                    0
                };
                if(execve("/usr/bin/", args, NULL) == -1)
                {
                    fprintf(stderr, "Error running command in execve\n");
                    return -100;
                }

            } else
                wait(NULL);           
        }
        // ls
        else if ( strcmp(parsedinput, "ls") == 0 ) {
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
        // date command       
        else if ( strcmp(parsedinput, "date") == 0 ) {
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
        else { // echo if no command
            pid_t forkV = fork();
            if ( forkV == 0 ) {
                args[0] = "/usr/bin/echo";
                args[1] = parsedinput;
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
        free(parsedinput);
    } while ( 1 );

    return 0;
}
