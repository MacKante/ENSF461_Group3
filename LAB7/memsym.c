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

/*----------------------- Global Variables -----------------------*/
// check if define() was called
u_int8_t isdefined;

// registers
u_int32_t r1 = 0;
u_int32_t r2 = 0;

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
    u_int8_t index;
    u_int32_t VPN;
    u_int32_t PFN;
    u_int8_t PID;

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

// number of bits
u_int8_t OFFbits;
u_int8_t VPNbits;
u_int8_t PFNbits;

/*----------- Forward Declarations -----------*/
void define(char* OFF, char* PFN, char* VPN);
void ctxswitch(char* PID);
void load(char* dst, char* src);
void store(char* dst, char* src);
void add();
void map(char* VPN, char* PFN);
void unmap(char* VPN);

u_int32_t rinspect(char* rN);

int8_t TLBCheck(u_int32_t VPN);
void defineCheck();
/*--------------------------- Do not touch ---------------------------*/
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

    isdefined = 0;
    
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
        if(strcmp(tokens[0], "define") == 0){
            define(tokens[1], tokens[2], tokens[3]);
            isdefined = 1;
        }

        if (strcmp(tokens[0], "ctxswitch") == 0){
            defineCheck();
            ctxswitch(tokens[1]);
        }
        
        if(strcmp(tokens[0], "load") == 0) { 
            defineCheck();
            load(tokens[1], tokens[2]);
        }
        
        if(strcmp(tokens[0], "store") == 0) {
            defineCheck();
            store(tokens[1], tokens[2]);
        }

        if(strcmp(tokens[0], "add") == 0) {
            defineCheck();
            add();
        }

        if(strcmp(tokens[0], "map") == 0) {
            defineCheck();
            map(tokens[1], tokens[2]);
        }

        if(strcmp(tokens[0], "unmap") == 0) {
            defineCheck();
            unmap(tokens[1]);
        }

        if(strcmp(tokens[0], "rinspect") == 0) {
            defineCheck();
            rinspect(tokens[1]);
        }
        // more functs here


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


/*--------------------------- Our functions ---------------------------*/


void define(char* OFF, char* PFN, char* VPN){
    if(isdefined == 1){     // check for defined 
        fprintf(output_file, "Current PID: %d. Error: multiple calls to define in the same trace\n", currentProcess->PID);
        exit(1);
    } else {
        int off = atoi(OFF);
        int pfn = atoi(PFN);
        int vpn = atoi(VPN);

        // set the number of bits in global variables
        OFFbits = off;
        PFNbits = pfn;
        VPNbits = vpn;

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

        fprintf(output_file, "Current PID: %d. Memory instantiation complete. OFF bits: %d. PFN bits: %d. VPN bits: %d\n",
                currentProcess->PID, off, pfn, vpn);
    }
    return;
}

void ctxswitch(char* PID) {
    u_int8_t pid = atoi(PID);
    
    if(pid < 0 || pid > 3){
        fprintf(output_file, "Current PID: %d. Invalid context switch to process %d\n", currentProcess->PID, pid);
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
        fprintf(output_file, "Current PID: %d. Switched execution context to process: %d\n", currentProcess->PID, pid);
    }

    return;
}

void load(char* dst, char* src) {
    
    u_int32_t* dest;
    if(strcmp(dst, "r1") == 0){
        dest = &r1;
    } else if(strcmp(dst, "r2") == 0){
        dest = &r2;
    } else {
        fprintf(output_file,"Current PID: %d. Error: invalid register operand %s\n", currentProcess->PID, dst);
        exit(1);
    }

    if (src[0] == '#' && src[1] != '\0') { // load immediate <dst> into  register <dst>
        *dest = atoi(src + 1);
        fprintf(output_file, "Current PID: %d. Loaded immediate %d into register %s\n", currentProcess->PID, *dest, dst);

    } else { // load the value of memory location <src> into register <dst>
        
        u_int32_t index;
        u_int32_t source = atoi(src);

        u_int32_t vpn = source >> OFFbits;
        u_int32_t offset = 0xFFFF >> (32-OFFbits);

        int8_t TLBindex = TLBCheck(vpn);
        
        // first check if translation is in TLB
        if (TLBindex == -1) {  // TLB miss
            fprintf(output_file, "Translating. Lookup for VPN <VPN> caused a TLB miss\n");  
            

        } else {               // TLB Hit
            fprintf(output_file, "Translating. Lookup for VPN %d hit in TLB entry %d. PFN is %d\n", vpn, TLBindex, TLB[TLBindex].PFN);
            index = offset & (TLB[TLBindex].PFN << OFFbits);
            *dest = pMem[index];
            fprintf(output_file, "Current PID: %d. Loaded value of location %s ( %u ) into register %s\n", currentProcess->PID, src+1, pMem[index], dst);
        }    
    }
    return;
}

void store(char* dst, char* src){
    u_int32_t* s;
    if(src[0] == '#'){ // if immediate
        src++;
        u_int32_t source = atoi(src);
        pMem[atoi(dst)] = source;      // store immediate in memory location
        fprintf(output_file, "Current PID: %d. Stored immediate %s into location %s\n", currentProcess->PID, src, dst);
        return;

    } else if(strcmp(src, "r1") == 0){    // if register r1
        s = &r1;

    } else if(strcmp(src, "r2") == 0){    // if register r2
        s = &r2;
        
    } else {
        fprintf(output_file,"Current PID: %d. Error: invalid register operand %s\n", currentProcess->PID, src);
        exit(1);
    }
    
    pMem[atoi(dst)] = *s;      // store value of r1 in memory location
    
    return;
}

void add(){
    // Add the values in registers r1 and r2 and stores the result in r1
    r1 += r2;
    fprintf(output_file, "Current PID: %d. Added contents of registers r1 (%d) and r2 (%d). Result: %d\n", currentProcess->PID, r1 - r2, r2, r1);
    return;
}

void map(char* VPN, char* PFN) {
    if(strcmp(strategy, "FIFO")){
        
    }

    u_int32_t vpn = atoi(VPN);
    u_int32_t pfn = atoi(PFN);

    // set current process pfn to new PFN
    currentProcess->PFN = pfn;

    // check if VPN mapping already exists, if so then overwrite it
    for (int i = 0; i < 8; i++) {
        if (TLB[i].VPN == vpn) {
            TLB[i].PFN = pfn;
            fprintf(output_file, "Current PID: %d. Mapped virtual page number %d to physical frame number %d\n", currentProcess->PID, vpn, pfn);
            return;
        }
    }

    // find first invalid TLB entry
    for (int i = 0; i < 8; i++) {
        if (TLB[i].valid == 0) {
            TLB[i].valid = 1;
            TLB[i].PFN = pfn;
            TLB[i].VPN = vpn;
            fprintf(output_file, "Current PID: %d. Mapped virtual page number %d to physical frame number %d\n", currentProcess->PID, vpn, pfn);
            return;
        }
    }

    // otherwise just use the first object
    TLB[0].valid = 1;
    TLB[0].PFN = pfn;
    TLB[0].VPN = vpn;
    fprintf(output_file, "Current PID: %d. Mapped virtual page number %d to physical frame number %d\n", currentProcess->PID, vpn, pfn);
}

void unmap(char* VPN){
    u_int32_t vpn = atoi(VPN);

    for (int i = 0; i < 8; i++) {
        if (TLB[i].VPN == vpn) {
            TLB[i].PFN = 0;
            TLB[i].valid = 0;
        }
    }

    fprintf(output_file, "Current PID: %d. Unmapped virtual page number %d\n", currentProcess->PID, vpn);
}

u_int32_t rinspect(char* rN) {
    if(strcmp(rN, "r1")== 0){
        fprintf(output_file, "Current PID: %d. Inspected register %s. Content: %d\n", currentProcess->PID, rN, r1);
        return r1;

    } else if(strcmp(rN, "r2") == 0){
        fprintf(output_file, "Current PID: %d. Inspected register %s. Content: %d\n", currentProcess->PID, rN, r2);
        return r2;

    } else {
        fprintf(output_file,"Current PID: %d. Error: invalid register operand %s\n", currentProcess->PID, rN);
        return 0 ;
    }
}

/*----------------------------- shared function calls -----------------------------*/
void defineCheck() {
    if (isdefined == 0) {
        u_int8_t pid;

        if (currentProcess == NULL) {
            pid = 0;
        } else {
            pid = currentProcess->PID;
        }

        fprintf(output_file, "Current PID: %d. Error: attempt to execute instruction before define\n", pid);
        exit(1);
    } else {
        return;
    }
}

int8_t TLBCheck(u_int32_t VPN) {
    // return index if TLB hit, -1 if miss
    for(int i = 0; i < 8; i++) {
        if (TLB[i].VPN == VPN) {
            return i;
        }
    }
    return -1;
}