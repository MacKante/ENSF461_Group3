#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <math.h>

#define TRUE 1
#define FALSE 0

// Output file
FILE* output_file;

// TLB replacement strategy (FIFO or LRU)
char* strategy;

// Globals
int isdefined = 0;

u_int32_t r1;
u_int32_t r2;

char** tokenize_input(char* input) {
    char** tokens = NULL;
    char* token = strtok(input, " ");
    int num_tokens = 0;

    while (token != NULL) {
        num_tokens++;
        tokens = realloc(tokens, num_tokens * sizeof(char*));
        tokens[num_tokens - 1] = malloc(strlen(token) + 1);
        strcpy(tokens[num_tokens - 1], token);
        token = strtok(NULL, " ");
    }

    num_tokens++;
    tokens = realloc(tokens, num_tokens * sizeof(char*));
    tokens[num_tokens - 1] = NULL;

    return tokens;
}

int main(int argc, char* argv[]) {
    const char usage[] = "Usage: memsym.out <strategy> <input trace> <output trace>\n";
    char* input_trace;
    char* output_trace;
    char buffer[1024];

    // Parse command line arguments
    if (argc != 4) {
        printf("%s", usage);
        return 1;
    }
    strategy = argv[1];
    input_trace = argv[2];
    output_trace = argv[3];

    // Open input and output files
    FILE* input_file = fopen(input_trace, "r");
    output_file = fopen(output_trace, "w");  

    while ( !feof(input_file) ) {
        // Read input file line by line
        char *rez = fgets(buffer, sizeof(buffer), input_file);
        if ( !rez ) {
            fprintf(stderr, "Reached end of trace. Exiting...\n");
            return -1;
        } else {
            // Remove endline character
            buffer[strlen(buffer) - 1] = '\0';
        }
        char** tokens = tokenize_input(buffer);

        // TODO: Implement your memory simulator
        if (strcmp(strategy, "define") == 0){
            
            //define();     // Call define
            isdefined += 1;
        }

        else if (isdefined == 0){       // if define has not been called, exit with error
            perror("Error: attempt to execute instruction before define");
            exit(1);
        } else if(isdefined == 1){      // if define has been called once, do it
            // Do it
        }

        // Deallocate tokens
        for (int i = 0; tokens[i] != NULL; i++)
            free(tokens[i]);
        free(tokens);
    }

    // Close input and output files
    fclose(input_file);
    fclose(output_file);

    return 0;
}

void* define(int OFF, int PFN, int VPN){
    if(isdefined >= 1){     // check for defined 
        perror("Error: multiple calls to define in the same trace\n");
        exit(1);
    } else {
        int size = pow(2, (OFF + PFN));
        u_int32_t arr[size];

        for (int i = 0; i < size; i++){
            arr[i] = 0;
        }
        
        printf("Memory instantiation complete. OFF bits: %d. PFN bits: %d. VPN bits: %d\n",
                OFF, PFN, VPN);

        return arr;
    }
}

void* ctxswitch(pid_t pid){
// sets the pid as the one currently executprintf("Memory instantiation complete. OFF bits: %d. PFN bits: %d. VPN bits: %d",
//     All memory translations
// and mapping should refer to the currently active process
// at the beginning the active process should be process 0
// When context switching, the current values of registers should be saved so
// that it can be restored when the process is scheduled again
// • It must output:
// Switched execution context to process <pid>



}

void* map(int VPN, int PFN){
}

void* unmap(int VPN){

}

void* pinspect(int VPN){

}

void* tinspect(int TLBN){

}

void* linspect(u_int32_t PL){

}

void* rinspect(char* rN){

//    printf("Inspected register <%s>. Content: %d", rN, content);
    u_int32_t *reg;
    if(strcmp(rN, "r1") == 0){
        reg = &r1;
    } else if(strcmp(rN, 'r2') == 0){
        reg = &r2;
    }
    else{
        printf("Unknown register is passed in");
        exit(1);
    }
}

u_int32_t load(char* rN){
    u_int32_t *reg;
    // checks to see the value of RN and see if its valid
    if(strcmp(rN, "r1") == 0){
        reg = &r1;
    } else if(strcmp(rN, 'r2') == 0){
        reg = &r2;
    }
    else{
        printf("Unknown register is passed");
        exit(1);
    }
    
    // loads the value
    
}
