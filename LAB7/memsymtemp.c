#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <math.h>

#define TRUE 1
#define FALSE 0

// Pre-defined Functions
void define(char* OFF, char* PFN, char* VPN);
void ctxswitch(char* PID);

// Output file
FILE* output_file;

// TLB replacement strategy (FIFO or LRU)
char* strategy;

/*----------------------- Global Variables -----------------------*/
int isdefined = 0;

// registers
u_int32_t r1;
u_int32_t r2;

// For storing register values when context switching
// Each process saves the values of r1 and r2
typedef struct {
    u_int32_t r1Saved;
    u_int32_t r2Saved;
} processReg;
// array of saved register values for context switching
processReg regCache[4];

// physical memory
u_int32_t* pMem;           

/*
Protection Bits Index
    0 : none
    1 : read
    2 : write
    3 : both
*/
// Structure Definitions
// TLB Entry
typedef struct {
    u_int32_t VPN;
    u_int32_t PFN;
    u_int8_t valid;
} TLBEntry;
// TLB
TLBEntry TLB[8];

// Page Table Entry
typedef struct {
    u_int8_t valid;
    u_int8_t PID;    
    u_int32_t PFN;
    u_int8_t protect;
} PTEntry;

// PageTable
PTEntry PageTable[4];

// current process
PTEntry* currentProcess;

// // Prefixed pid print
// #define PREFIX "Current PID: %d. "
// #define fprintf(output_file, format, ...) fprintf(output_file, PREFIX format, currentProcess->PID, ##__VA_ARGS__)

/*------------------------- BASE CODE -------------------------*/
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
            printf("Reached end of trace. Exiting...\n");
            return -1;
        } else {
            // Remove endline character
            buffer[strlen(buffer) - 1] = '\0';
        }
        char** tokens = tokenize_input(buffer);
        
            fprintf(output_file, "%s\n", tokens[i]);

        // TODO: Implement your memory simulator
        if (strcmp(tokens[0], "define") == 0){
            define(tokens[1], tokens[2], tokens[3]);     // Call define
            isdefined += 1;

        } else if (isdefined == 0){                     // if define has not been called, exit with error
            perror("Error: attempt to execute instruction before define");
            exit(1);

        } else if(isdefined == 1){      // if define has been called once, do it
            // if (strcmp(tokens[0], "ctxswitch") == 0){
            //     ctxswitch(tokens[1]);      // Call ctxswitch
            // }
            // if (strcmp(tokens[0], "map") == 0){
            //     map(tokens[1], tokens[2]);
            // }
            // if (strcmp(tokens[0], "unmap") == 0){
            //     unmap(tokens[1]);
            // }
            // if (strcmp(tokens[0], "pinspect") == 0){
            //     pinspect(tokens[1]);
            // }
            // if (strcmp(tokens[0], "tinspect") == 0){
            //     tinspect(tokens[1]);
            // }
            // if (strcmp(tokens[0], "linspect") == 0){
            //     linspect(tokens[1]);
            // }
            // if (strcmp(tokens[0], "rinspect") == 0){
            //     rinspect(tokens[1]);
            // }
            // if (strcmp(tokens[0], "load") == 0){
            //     load(tokens[1], tokens[2]);
            // }
            // if (strcmp(tokens[0], "store") == 0){
            //     store(tokens[1], tokens[2]);
            // }
            // if (strcmp(tokens[0], "add") == 0){
            //     add();
            // }
            
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

/*---------------------- OUR FUNCTIONS ----------------------*/

void define(char* OFF, char* PFN, char* VPN){
    if(isdefined != 0){     // check for defined 
        fprintf(output_file, "Current PID: %d. Error: multiple calls to define in the same trace\n", currentProcess->PID);
        exit(1);
    } else {
        int off = atoi(OFF);
        int pfn = atoi(PFN);
        int vpn = atoi(VPN);

        // Set Physical Memory
        int psize = (int)pow((double)2, (double)(off + pfn));
        pMem = (u_int32_t*)calloc(psize, sizeof(u_int32_t));
        
        // Initialize page table entries
        for(int i = 0; i < 4; i++) {
            PageTable[i].valid = 0;
            PageTable[i].PID = i;
        }

        // Initialize Current 
        currentProcess = &PageTable[0];

        fprintf(output_file, "Current PID: %d. Memory instantiation complete. OFF bits: %d. PFN bits: %d. VPN bits: %d \n",
                currentProcess->PID, off, pfn, vpn);
    }
    return;
}

void ctxswitch(char* PID){
    u_int8_t pid = atoi(PID);
    
    if(pid < 0 || pid > 3){
        fprintf(output_file, "Invalid context switch to process %d\n", pid);
        exit(1);
        
    } else {
        // Saving current process's registers
        regCache[currentProcess->PID].r1Saved = r1;
        regCache[currentProcess->PID].r2Saved = r2;

        // Restoring Registers to State of process with pid 
        r1 = regCache[pid].r1Saved;
        r2 = regCache[pid].r2Saved;
        
        // set current process to the one specified by PID argument
        currentProcess = &PageTable[pid];
        fprintf(output_file, "Switched execution context to process %d\n", pid);
    }

    return;
}

void map(char* VPN, char* PFN) {
    u_int32_t vpn = atoi(VPN);
    u_int32_t pfn = atoi(PFN);

    // set current process pfn to new PFN
    currentProcess->PFN = pfn;

    // check if VPN mapping already exists, if so then overwrite it
    for (int i = 0; i < 8; i++) {
        if (TLB[i].VPN == vpn) {
            TLB[i].PFN = pfn;
            fprintf(output_file, "Mapped virtual page number %d to physical frame number %d\n", vpn, pfn);
            return;
        }
    }

    // find first invalid TLB entry
    for (int i = 0; i < 8; i++) {
        if (TLB[i].valid == 0) {
            TLB[i].valid = 1;
            TLB[i].PFN = pfn;
            TLB[i].VPN = vpn;

            fprintf(output_file, "Mapped virtual page number %d to physical frame number %d\n", vpn, pfn);
            return;
        }
    }

    // otherwise just use the first object
    TLB[0].valid = 1;
    TLB[0].PFN = pfn;
    TLB[0].VPN = vpn;
    fprintf(output_file, "Mapped virtual page number %d to physical frame number %d\n", vpn, pfn);
}

void unmap(char* VPN){
    u_int32_t vpn = atoi(VPN);
    for (int i = 0; i < 8; i++) {
        if (TLB[i].VPN == vpn) {
            TLB[i].PFN = 0;
            TLB[i].valid = 0;
        } 
    }
}

// void pinspect(char* VPN){

// }

// void tinspect(char* TLBN){

// }

// void linspect(char* PL){

// }

// void rinspect(char* rN){

// }

void load(char* dst, char* src){
    u_int32_t* dest;

    if(strcmp(dst, "r1") == 0){
        dest = &r1;
    } else if(strcmp(dst, "r2") == 0){
        dest = &r2;
    } else {
        ffprintf(output_file, stderr,"Error: invalid register operand %s\n", dst);
        exit(1);
    }

    if (src[0] == '#' && src[1] != '\0') {
        // load immediate <dst> into  register <dst>
        *dest = atoi(src + 1);
        fprintf(output_file, "Loaded immediate %s into register %s", src, dst);
    } else {
        u_int32_t loadedValue;
        // load the value of memory location <src> into register <dst>
        /* loadedValue = Memory Location Value ??? */
        fprintf(output_file, "Loaded value of location %s ( %u ) into register %s", src, loadedValue, dest);
    }

    return;
}

void store(char* dst, char* src){
    u_int32_t* s;
    u_int32_t source;
    
    if(src[0] == "#"){ // if immediate
        src++;
        u_int32_t source = atoi(src);
        // memory_location = source;      // store immediate in memory location
        fprintf(output_file, "Stored immediate %s into location %s\n", src, dst);
        return;

    } else if(strcmp(src, "r1") == 0){    // if register r1
        s = &r1;

    } else if(strcmp(src, "r2") == 0){    // if register r2
        s = &r2;
        
    } else {
        ffprintf(output_file, stderr,"Error: invalid register operand %s\n", src);
        exit(1);
    }

    // memory_location = *s;      // store value of r1 in memory location
    
    return;
}

void add(){
    // Add the values in registers r1 and r2 and stores the result in r1
    r1 += r2;
    fprintf(output_file, "Added register r1 (%d) to register r2 (%d). Result: %d\n", r1 - r2, r2, r1);
    return;
}