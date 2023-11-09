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
int isdefined=0;

// // registers
// u_int32_t r1 = 0;
// u_int32_t r2 = 0;

// For storing register values when context switching
// Each process saves the values of r1 and r2
// PROCESS REGISTERS
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
    // u_int8_t index;
    u_int32_t VPN;
    u_int32_t PFN;
    int PID;

    int valid;
    u_int32_t timestamp; // TIME STAMPS
} TLBEntry;
// TLB
TLBEntry TLB[8];


// Page Table Entry
typedef struct {
    int valid;
    // u_int8_t PID;
    u_int32_t PFN;
    // u_int8_t protect; // what is this for
    
    

} PTEntry;

// PageTable
PTEntry* PageTable[4];

// current process
int currentProcess=0;

// number of bits
int OFFbits;
int VPNbits;
int PFNbits;
u_int32_t globalTime = 0;

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

void pinspect(char * srt){
    int vpn = atoi(srt);
    int pfn = PageTable[currentProcess][vpn].PFN;
    int valid = PageTable[currentProcess][vpn].valid;
    fprintf(output_file, "Current PID: %d. Inspected page table entry %d. Physical frame number: %d. Valid: %d\n",
        currentProcess, vpn, pfn, valid);

}
void tinspect(char * srt){
    int tlbn = atoi(srt);
    fprintf(output_file, "Current PID: %d. Inspected TLB entry %d. VPN: %d. PFN: %d. Valid: %d. PID: %d. Timestamp: %u\n",
        currentProcess, tlbn, TLB[tlbn].VPN, TLB[tlbn].PFN, TLB[tlbn].valid, TLB[tlbn].PID, TLB[tlbn].timestamp);

}
void linspect(char * srt){
    u_int32_t pl = strtoul(srt, NULL, 10);
    fprintf(output_file, "Current PID: %d. Inspected physical location %d. Value: %u\n",
        currentProcess, pl, pMem[pl]);

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
        else if(isdefined==0){
            if(strcmp(tokens[0],"%")!=0 && strcmp(tokens[0],"")!=0 ){
            fprintf(output_file,"Current PID: %d. Error: attempt to execute instruction before define\n",currentProcess);
            exit(1);}
        }

        else if (strcmp(tokens[0], "ctxswitch") == 0){
            
            ctxswitch(tokens[1]);
        }
        
        else if(strcmp(tokens[0], "load") == 0) {
           
            load(tokens[1], tokens[2]);
        }
        
        else if(strcmp(tokens[0], "store") == 0) {
            
            store(tokens[1], tokens[2]);
        }

        else if(strcmp(tokens[0], "add") == 0) {
            
            add();
        }

        else if(strcmp(tokens[0], "map") == 0) {
            
            map(tokens[1], tokens[2]);
        }

       else if(strcmp(tokens[0], "unmap") == 0) {
             
            unmap(tokens[1]);
        }

        else if(strcmp(tokens[0], "rinspect") == 0) {
             
            rinspect(tokens[1]);
        }
        // more functs here
        else if (strcmp(tokens[0], "pinspect") == 0){
            pinspect(tokens[1]);
        }
        else if (strcmp(tokens[0], "tinspect") == 0){
            tinspect(tokens[1]);
        }
        else if (strcmp(tokens[0], "linspect") == 0){
            linspect(tokens[1]);
        }

        // Deallocate tokens
        for (int i = 0; tokens[i] != NULL; i++)
            free(tokens[i]);
        free(tokens);
        globalTime++;
    }

    // Close input and output files
    fclose(input_file);
    fclose(output_file);

    return 0;
}


/*--------------------------- Our functions ---------------------------*/


void define(char* OFF, char* PFN, char* VPN){
    if(isdefined == 1){     // check for defined
        fprintf(output_file, "Current PID: %d. Error: multiple calls to define in the same trace\n", currentProcess);
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
        // int psize = (int)pow((double)2, (double)(off + pfn));
        int psize = 1 << (off + pfn);

        pMem = (u_int32_t*)calloc(psize, sizeof(u_int32_t));

        // Initialize TLB entries
        for(int i=0; i<8;i++){
            TLB[i].valid = 0;
            TLB[i].PFN = 0;
            TLB[i].VPN = 0;
            TLB[i].PID = -1;
            TLB[i].timestamp = 0;
        }
        // Initialize page table entries
        for(int i = 0; i < 4; i++) {

            PageTable[i] = (PTEntry*)calloc(1 << vpn, sizeof(PTEntry));
            regCache[i] = (processReg){0,0};
        }


        // Initialize Current
        // currentProcess = &PageTable[0];

        fprintf(output_file, "Current PID: %d. Memory instantiation complete. OFF bits: %d. PFN bits: %d. VPN bits: %d\n",
                currentProcess, off, pfn, vpn);
        
    }
    return;
}

void ctxswitch(char* PID) {
    int pid = atoi(PID);
    
    if(pid < 0 || pid > 3){
        fprintf(output_file, "Current PID: %d. Invalid context switch to process %d\n", currentProcess, pid);
        exit(1);
    } else {

        
        // set current process to the one specified by PID argument
        currentProcess = pid;
        fprintf(output_file, "Current PID: %d. Switched execution context to process: %d\n", currentProcess, pid);
    }

    return;
}

void load(char* dst, char* src) {
    u_int32_t localPFN;
    u_int32_t s;
    u_int32_t register_value;
    if(strcmp(dst, "r1") != 0  && (strcmp(dst, "r2") != 0)){
        fprintf(output_file,"Current PID: %d. Error: invalid register operand %s\n", currentProcess, dst);
        exit(1);
    }

    if (src[0] == '#') { // load immediate <dst> into  register <dst>
        if(src[1]=='\0'){
            fprintf(output_file, "Current PID: %d. ERROR: Invalid immediate\n", currentProcess);
            exit(1);
        }
        if(strcmp(dst, "r1") == 0){
            register_value=regCache[currentProcess].r1Saved = atoi(src + 1);
            
            }
        else{
            register_value=regCache[currentProcess].r2Saved = atoi(src + 1);
        }

        fprintf(output_file, "Current PID: %d. Loaded immediate %d into register %s\n", currentProcess, register_value, dst);

    } else { // load the value of memory location <src> into register <dst>
        u_int32_t VPN = strtoul(src, NULL, 10);
        u_int32_t OFFsetValue = 0xFFFFFFFF >> (32-OFFbits);
        OFFsetValue = VPN & OFFsetValue ;
        VPN = VPN >>OFFbits;
        int i;
        for(i=0; i<8; i++){
            if(TLB[i].VPN == VPN && TLB[i].PID == currentProcess){
                break;
            }
        }
        if(i!=8){
            if(strcmp(strategy, "LRU") == 0){
                TLB[i].timestamp = globalTime;
            }
            fprintf(output_file, "Current PID: %d. Translating. Lookup for VPN %d hit in TLB entry %d. PFN is %d\n", currentProcess, VPN, i, TLB[i].PFN);
            localPFN = TLB[i].PFN;
        }
        else{
            fprintf(output_file, "Current PID: %d. Translating. Lookup for VPN %d caused a TLB miss\n", currentProcess, VPN);
            
            if(PageTable[currentProcess][VPN].valid == 0){
                
                fprintf(output_file, "Current PID: %d. Translating. Translation for VPN %d not found in page table\n", currentProcess, VPN);
                exit(1);
            }
            localPFN = PageTable[currentProcess][VPN].PFN;

                // for (int i = 0; i < 8; i++) {
                //     if (TLB[i].VPN == VPN&& TLB[i].PID == currentProcess ) {
                //         TLB[i]= (TLBEntry){ VPN, localPFN, currentProcess, 1, globalTime };
                //         fprintf(output_file, "Current PID: %d. Translating. Successfully mapped VPN %d to PFN %d\n", currentProcess, VPN, localPFN);
                //         return;
                //     }
                // }
                // find first invalid TLB entry
                for (int i = 0; i < 8; i++) {
                    if (TLB[i].valid == 0) {
                        TLB[i]= (TLBEntry){ VPN, localPFN, currentProcess, 1, globalTime };
                        fprintf(output_file, "Current PID: %d. Mapped virtual page number %d to physical frame number %d\n", currentProcess, VPN, localPFN);
                        return;
                    }
                }
                int min_timestamp_index = 0;
                for(i=0; i<8; i++){
                    if(TLB[i].timestamp < TLB[min_timestamp_index].timestamp){
                        min_timestamp_index = i;
                    }
                }
                i=min_timestamp_index;
                TLB[i]= (TLBEntry){ VPN, localPFN, currentProcess, 1, globalTime};
                fprintf(output_file, "Current PID: %d. Translating. Successfully mapped VPN %d to PFN %d\n", currentProcess, VPN, localPFN);
        }

        if(strcmp(dst, "r1") == 0){
            s=regCache[currentProcess].r1Saved = pMem[(localPFN<<OFFbits)+OFFsetValue];
        }
        else{
            s=regCache[currentProcess].r2Saved = pMem[(localPFN<<OFFbits)+OFFsetValue];
        }

            // store value of r1 in memory location
        fprintf(output_file, "Current PID: %d. Loaded value of location %s (%d) into register %s\n", currentProcess, src,s, dst);
        

    }
    return;
}


void store(char* dst, char* src){
    u_int32_t s;
    u_int32_t localPFN;

    if(src[0] == '#'){ // if immediate
    if(src[1]=='\0'){
            fprintf(output_file, "Current PID: %d. ERROR: Invalid immediate\n", currentProcess);
            exit(1);
        }
        int VPN = atoi(dst);
        int OFFsetValue = 0xFFFFFFFF >> (32-OFFbits);
        OFFsetValue = VPN & OFFsetValue ;
        VPN = VPN >>OFFbits;
        int i;
        for(i=0; i<8; i++){
            if(TLB[i].VPN == VPN && TLB[i].PID == currentProcess){
                break;
            }
        }
        if(i!=8){
            if(strcmp(strategy, "LRU") == 0){
                TLB[i].timestamp = globalTime;
            }
            fprintf(output_file, "Current PID: %d. Translating. Lookup for VPN %d hit in TLB entry %d. PFN is %d\n", currentProcess, VPN, i, TLB[i].PFN);
            localPFN = TLB[i].PFN;
        }
        else{
            fprintf(output_file, "Current PID: %d. Translating. Lookup for VPN %d caused a TLB miss\n", currentProcess, VPN);
            
            if(PageTable[currentProcess][VPN].valid == 0){
                
                fprintf(output_file, "Current PID: %d. Translating. Translation for VPN %d not found in page table\n", currentProcess, VPN);
                exit(1);
            }
            localPFN = PageTable[currentProcess][VPN].PFN;


                // find first invalid TLB entry
                for (int i = 0; i < 8; i++) {
                    if (TLB[i].valid == 0) {
                        TLB[i]= (TLBEntry){ VPN, localPFN, currentProcess, 1, globalTime };
                        fprintf(output_file, "Current PID: %d. Mapped virtual page number %d to physical frame number %d\n", currentProcess, VPN, localPFN);
                        return;
                    }
                }
                int min_timestamp_index = 0;
                for(i=0; i<8; i++){
                    if(TLB[i].timestamp < TLB[min_timestamp_index].timestamp){
                        min_timestamp_index = i;
                    }
                }
                i=min_timestamp_index;
                TLB[i]= (TLBEntry){ VPN, localPFN, currentProcess, 1, globalTime};
                fprintf(output_file, "Current PID: %d. Translating. Successfully mapped VPN %d to PFN %d\n", currentProcess, VPN, localPFN);
        }
        src = src + 1;
        u_int32_t source = strtoul(src, NULL, 10);
        u_int32_t physLoc = (localPFN<<OFFbits)|OFFsetValue;
        pMem[physLoc] = source;      // store immediate in memory location
        fprintf(output_file, "Current PID: %d. Stored immediate %s into location %s\n", currentProcess, src, dst);
        return;

    }

    else if(strcmp(src, "r1") == 0){    // if register r1
        
        u_int32_t VPN = strtoul(dst, NULL, 10);
        u_int32_t OFFsetValue = 0xFFFFFFFF >> (32-OFFbits);
        OFFsetValue = VPN & OFFsetValue ;
        VPN = VPN >>OFFbits;
        int i;
        for(i=0; i<8; i++){
            if(TLB[i].VPN == VPN && TLB[i].PID == currentProcess){
                break;
            }
        }
        if(i!=8){
            if(strcmp(strategy, "LRU") == 0){
                TLB[i].timestamp = globalTime;
            }
            fprintf(output_file, "Current PID: %d. Translating. Lookup for VPN %d hit in TLB entry %d. PFN is %d\n", currentProcess, VPN, i, TLB[i].PFN);
            localPFN = TLB[i].PFN;
        }
        else{
            fprintf(output_file, "Current PID: %d. Translating. Lookup for VPN %d caused a TLB miss\n", currentProcess, VPN);
            
            if(PageTable[currentProcess][VPN].valid == 0){
                
                fprintf(output_file, "Current PID: %d. Translating. Translation for VPN %d not found in page table\n", currentProcess, VPN);
                exit(1);
            }
            localPFN = PageTable[currentProcess][VPN].PFN;

                // for (int i = 0; i < 8; i++) {
                //     if (TLB[i].VPN == VPN&& TLB[i].PID == currentProcess ) {
                //         TLB[i]= (TLBEntry){ VPN, localPFN, currentProcess, 1, globalTime };
                //         fprintf(output_file, "Current PID: %d. Translating. Successfully mapped VPN %d to PFN %d\n", currentProcess, VPN, localPFN);
                //         return;
                //     }
                // }
                // find first invalid TLB entry
                for (int i = 0; i < 8; i++) {
                    if (TLB[i].valid == 0) {
                        TLB[i]= (TLBEntry){ VPN, localPFN, currentProcess, 1, globalTime };
                        fprintf(output_file, "Current PID: %d. Mapped virtual page number %d to physical frame number %d\n", currentProcess, VPN, localPFN);
                        return;
                    }
                }
                int min_timestamp_index = 0;
                for(i=0; i<8; i++){
                    if(TLB[i].timestamp < TLB[min_timestamp_index].timestamp){
                        min_timestamp_index = i;
                    }
                }
                i=min_timestamp_index;
                TLB[i]= (TLBEntry){ VPN, localPFN, currentProcess, 1, globalTime};
                fprintf(output_file, "Current PID: %d. Translating. Successfully mapped VPN %d to PFN %d\n", currentProcess, VPN, localPFN);
        }

         


        s = regCache[currentProcess].r1Saved;
        u_int32_t physLoc = (localPFN<<OFFbits)|OFFsetValue;
        pMem[physLoc] = s;    // store value of r1 in memory location
        fprintf(output_file, "Current PID: %d. Stored value of register %s (%d) into location %s\n", currentProcess, src,s, dst);
    }
    else if(strcmp(src, "r2") == 0){    // if register r2
        int VPN = strtoul(dst, NULL, 10);
        int OFFsetValue = 0xFFFFFFFF >> (32-OFFbits);
        OFFsetValue = VPN & OFFsetValue ;
        VPN = VPN >>OFFbits;
        int i;
        for(i=0; i<8; i++){
            if(TLB[i].VPN == VPN && TLB[i].PID == currentProcess){
                break;
            }
        }
        if(i!=8){
            if(strcmp(strategy, "LRU") == 0){
                TLB[i].timestamp = globalTime;
            }
            fprintf(output_file, "Current PID: %d. Translating. Lookup for VPN %d hit in TLB entry %d. PFN is %d\n", currentProcess, VPN, i, TLB[i].PFN);
            localPFN = TLB[i].PFN;
        }
        else{
            fprintf(output_file, "Current PID: %d. Translating. Lookup for VPN %d caused a TLB miss\n", currentProcess, VPN);
            
            if(PageTable[currentProcess][VPN].valid == 0){
                
                fprintf(output_file, "Current PID: %d. Translating. Translation for VPN %d not found in page table\n", currentProcess, VPN);
                exit(1);
            }
            localPFN = PageTable[currentProcess][VPN].PFN;

                // for (int i = 0; i < 8; i++) {
                //     if (TLB[i].VPN == VPN&& TLB[i].PID == currentProcess ) {
                //         TLB[i]= (TLBEntry){ VPN, localPFN, currentProcess, 1, globalTime };
                //         fprintf(output_file, "Current PID: %d. Translating. Successfully mapped VPN %d to PFN %d\n", currentProcess, VPN, localPFN);
                //         return;
                //     }
                // }
                // find first invalid TLB entry
                for (int i = 0; i < 8; i++) {
                    if (TLB[i].valid == 0) {
                        TLB[i]= (TLBEntry){ VPN, localPFN, currentProcess, 1, globalTime };
                        fprintf(output_file, "Current PID: %d. Mapped virtual page number %d to physical frame number %d\n", currentProcess, VPN, localPFN);
                        return;
                    }
                }
                int min_timestamp_index = 0;
                for(i=0; i<8; i++){
                    if(TLB[i].timestamp < TLB[min_timestamp_index].timestamp){
                        min_timestamp_index = i;
                    }
                }
                i=min_timestamp_index;
                TLB[i]= (TLBEntry){ VPN, localPFN, currentProcess, 1, globalTime};
                fprintf(output_file, "Current PID: %d. Translating. Successfully mapped VPN %d to PFN %d\n", currentProcess, VPN, localPFN);
        }

        s = regCache[currentProcess].r2Saved;
        pMem[(localPFN<<OFFbits)+OFFsetValue] = s;
        fprintf(output_file, "Current PID: %d. Stored value of register %s (%d) into location %s\n", currentProcess, src,s, dst);
        
    } else {
        fprintf(output_file,"Current PID: %d. Error: invalid register operand %s\n", currentProcess, src);
        exit(1);
    }
    

  
    
    return;
}

void add(){
    // Add the values in registers r1 and r2 and stores the result in r1
    u_int32_t r1 = regCache[currentProcess].r1Saved;
    u_int32_t r2 = regCache[currentProcess].r2Saved;
    regCache[currentProcess].r1Saved = r1 + r2;
    fprintf(output_file, "Current PID: %d. Added contents of registers r1 (%u) and r2 (%u). Result: %u\n", currentProcess, r1, r2, r1+r2);
    return;
}

void map(char* VPN, char* PFN) {
    u_int32_t vpn = atoi(VPN);
    u_int32_t pfn = atoi(PFN);
    int i;
    // for(i =0; i<8; i++){
    //     if(TLB[i].VPN == vpn && TLB[i].PID == currentProcess){
    //         break;
    //     }
    // }
    // if(i == 8){
    //     for(i=0; i<8; i++){
    //         if(TLB[i].valid == 0){
    //             break;
    //         }
    //     }
    // }
    

    // // check if VPN mapping already exists, if so then overwrite it
    for (int i = 0; i < 8; i++) {
        if (TLB[i].VPN == vpn&& TLB[i].PID == currentProcess ) {
            TLB[i]= (TLBEntry){ vpn, pfn, currentProcess, 1, globalTime };
            PageTable[currentProcess][vpn].valid = 1;
            PageTable[currentProcess][vpn].PFN = pfn;
            fprintf(output_file, "Current PID: %d. Mapped virtual page number %d to physical frame number %d\n", currentProcess, vpn, pfn);

            return;
        }
    }

    // find first invalid TLB entry
    for (int i = 0; i < 8; i++) {
        if (TLB[i].valid == 0) {
            TLB[i]= (TLBEntry){ vpn, pfn, currentProcess, 1, globalTime };
            PageTable[currentProcess][vpn].valid = 1;
            PageTable[currentProcess][vpn].PFN = pfn;
            fprintf(output_file, "Current PID: %d. Mapped virtual page number %d to physical frame number %d\n", currentProcess, vpn, pfn);
            return;
        }
    }
    int min_timestamp_index = 0;
    for(i=0; i<8; i++){
        if(TLB[i].timestamp < TLB[min_timestamp_index].timestamp){
            min_timestamp_index = i;
        }
    }
    i=min_timestamp_index;
    TLB[i]= (TLBEntry){ vpn, pfn, currentProcess, 1, globalTime };
    PageTable[currentProcess][vpn].valid = 1;
    PageTable[currentProcess][vpn].PFN = pfn;
    fprintf(output_file, "Current PID: %d. Mapped virtual page number %d to physical frame number %d\n", currentProcess, vpn, pfn);

    return;
}

void unmap(char* VPN){
    u_int32_t vpn = atoi(VPN);

    for (int i = 0; i < 8; i++) {
        if (TLB[i].VPN == vpn && TLB[i].PID == currentProcess) {
            TLB[i].PFN = 0;
            TLB[i].valid = 0;
            TLB[i].VPN = 0;
            TLB[i].PID = -1;
            TLB[i].timestamp = 0;

        }
    }
    PageTable[currentProcess][vpn] = (PTEntry){0,0};

    fprintf(output_file, "Current PID: %d. Unmapped virtual page number %d\n", currentProcess, vpn);
}

u_int32_t rinspect(char* rN) {
    if(strcmp(rN, "r1")== 0){
        fprintf(output_file, "Current PID: %d. Inspected register %s. Content: %d\n", currentProcess, rN, regCache[currentProcess].r1Saved);
        return regCache[currentProcess].r1Saved;

    } else if(strcmp(rN, "r2") == 0){
        fprintf(output_file, "Current PID: %d. Inspected register %s. Content: %d\n", currentProcess, rN, regCache[currentProcess].r2Saved);
        return regCache[currentProcess].r2Saved;

    } else {
        fprintf(output_file,"Current PID: %d. Error: invalid register operand %s\n", currentProcess, rN);
        exit(-1);
        // return 0 ;
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
