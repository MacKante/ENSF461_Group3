#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>

#define TRUE 1
#define FALSE 0

int memory_initialized = FALSE;

int current_process = 0; // keep track of current process

int registers[2] = {0}; // keep track of current registers r1 and r2

uint32_t timestamp = 0; //global timestamp variable

struct ProcessState {
    int r1;
    int r2;
};

struct ProcessState process_states[4];

// TLB entry structure
struct TLBEntry {
    int valid;       // Indicates if the entry is valid
    int process_id;  // Process ID associated with the entry (0 to 4)
    int vpn;         // Virtual Page Number
    int pfn;         // Page Frame Number
    uint32_t timestamp;  //timestampt for fifo and lru strategies
};

// Page table entry structure
struct PageTableEntry {
    int valid;  // Indicates if the entry is valid
    int pfn;    // Page Frame Number
};

//array of page tables for each process
struct PageTableEntry** page_tables = NULL;

//physical memory
uint32_t* physical_memory = NULL;

// TLB 
struct TLBEntry tlb[8];

// Output file
FILE* output_file;

// TLB replacement strategy (FIFO or LRU)
char* strategy;

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

    //Initialize variables 
    int num_frames; 
    int num_pages;
    int off;

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
            //Increment timestamp if line is an instruction (not starting with %)
            if (buffer[0] != '%'){
                timestamp++;
            }
        }

        char** tokens = tokenize_input(buffer);

        if (strcmp(tokens[0], "define") == 0){
            //check if defined is calles more than once
            if (memory_initialized){
                fprintf(output_file, "Current PID: %d. Error: multiple calls to define in the same trace\n", current_process);
                return -1;
            }

            off = atoi(tokens[1]);
            int pfn = atoi(tokens[2]);
            int vpn_bits = atoi(tokens[3]);

            num_frames = 1 << (off + pfn);
            num_pages = 1 << vpn_bits;

            //initialize physical memory
            physical_memory = malloc(num_frames * sizeof(uint32_t));
            memset(physical_memory, 0, num_frames * sizeof(uint32_t));

            //initialize array of 4 page tables
            page_tables = malloc(4 * sizeof(struct PageTableEntry*));
            for (int i = 0; i < 4; i++) {
                page_tables[i] = malloc(num_pages * sizeof(struct PageTableEntry));
            }

            //initialize TLB entries as invalid
            for (int i = 0; i < 8; i++) {
                tlb[i].valid = FALSE;
                tlb[i].process_id = -1; // Initialize with an invalid process ID
            }

            //initialize page table entries as invalid for all processes
            for (int pid = 0; pid < 4; pid++) {
                for (int vpn = 0; vpn < num_pages; vpn++) {
                    page_tables[pid][vpn].valid = FALSE;
                }
            }

            //memory has been initialized
            memory_initialized = TRUE;

            fprintf(output_file, "Current PID: %d. Memory instantiation complete. OFF bits: %d. PFN bits: %d. VPN bits: %d\n", current_process, off, pfn, vpn_bits);
        } else if (tokens[0] == NULL){
            fprintf(output_file, "\n");
        } else if (strcmp(tokens[0], "ctxswitch") == 0){
            int new_pid = atoi(tokens[1]);

            //raise error if context swicth to invalid process
            if (new_pid < 0 || new_pid > 3){
                fprintf(output_file, "Current PID: %d. Invalid context switch to process %d\n", current_process, new_pid);
                return -1;
            }

            //save the current state of registers for the current process
            process_states[current_process].r1 = registers[0];
            process_states[current_process].r2 = registers[1];

            //change current process
            current_process = new_pid;

            //restore registers' values from the saved state if available
            registers[0] = process_states[current_process].r1;
            registers[1] = process_states[current_process].r2;

            fprintf(output_file, "Current PID: %d. Switched execution context to process: %d\n", current_process, new_pid);

        } else if (strcmp(tokens[0], "map") == 0){
            //check if memory is not initialized yet and raise error
            if (!memory_initialized) {
                fprintf(output_file, "Current PID: %d. Error: Memory not initialized\n", current_process);
                return -1;
            }

            //check if the current process is valid (0 to 3)
            if (current_process < 0 || current_process > 3) {
                fprintf(output_file, "Current PID: %d. Error: Invalid current process\n", current_process);
                return -1;
            }

            //parse VPN and PFN from tokens
            int vpn = atoi(tokens[1]);
            int pfn = atoi(tokens[2]);

            //check if VPN is within the valid range
            if (vpn < 0 || vpn >= num_pages) {
                fprintf(output_file, "Current PID: %d. Error: Invalid VPN %d\n", current_process, vpn);
                return -1;
            }

            //search for an existing TLB entry for the current process and VPN
            int tlb_entry_index = -1;
            for (int i = 0; i < 8; i++) {
                if (tlb[i].valid && tlb[i].process_id == current_process && tlb[i].vpn == vpn) {
                    tlb_entry_index = i;
                    break;
                }
            }

            //if an existing entry is found, update it
            //otherwise, find an empty entry
            if (tlb_entry_index != -1) {
                tlb[tlb_entry_index].pfn = pfn;
                if (strcmp(strategy, "FIFO") == 0){
                    tlb[tlb_entry_index].timestamp = timestamp;
                }
            } else {
                //find an empty TLB entry
                for (int i = 0; i < 8; i++) {
                    if (!tlb[i].valid) {
                        tlb_entry_index = i;
                        break;
                    }
                }

                // if all entries are occupied, implemnt either FIFO or LRU
                if (tlb_entry_index == -1) {
                    if (strcmp(strategy, "FIFO") == 0) {
                        //find TLB entry with the smallest timestamp.
                        int oldest_index = 0;
                        for (int i = 1; i < 8; i++) {
                            if (tlb[i].timestamp < tlb[oldest_index].timestamp) {
                                oldest_index = i;
                            }
                        }
                        tlb_entry_index = oldest_index;
                    } else if (strcmp(strategy, "LRU") == 0) {
                        //find TLB entry with the smallest timestamp.
                        int least_recent_index = 0;
                        for (int i = 1; i < 8; i++) {
                            if (tlb[i].timestamp < tlb[least_recent_index].timestamp) {
                                least_recent_index = i;
                            }
                        }
                        tlb_entry_index = least_recent_index;
                    }
                }

                //update TLB entry
                tlb[tlb_entry_index].valid = TRUE;
                tlb[tlb_entry_index].process_id = current_process;
                tlb[tlb_entry_index].vpn = vpn;
                tlb[tlb_entry_index].pfn = pfn;
                tlb[tlb_entry_index].timestamp = timestamp;
            }

            //update page table entry for current process and VPN
            page_tables[current_process][vpn].valid = TRUE;
            page_tables[current_process][vpn].pfn = pfn;

            fprintf(output_file, "Current PID: %d. Mapped virtual page number %d to physical frame number %d\n", current_process, tlb[tlb_entry_index].vpn, tlb[tlb_entry_index].pfn = pfn);
   
        } else if (strcmp(tokens[0], "unmap") == 0){
            //parse VPN from tokens
            int vpn = atoi(tokens[1]);

            //search for and invalidate the TLB entry for the current process and VPN
            for (int i = 0; i < 8; i++) {
                if (tlb[i].valid && tlb[i].process_id == current_process && tlb[i].vpn == vpn) {
                    tlb[i].valid = FALSE;
                    break;
                }
            }

            //invalidate the page table entry for the current process and VPN
            page_tables[current_process][vpn].valid = FALSE;

            fprintf(output_file, "Current PID: %d. Unmapped virtual page number %d\n", current_process, vpn);

        } else if (strcmp(tokens[0], "store") == 0){

            char* dst_operand = tokens[1];
            char* src_operand = tokens[2];
            int dst_virtual_address;
            int src_value;

            //parse the destination virtual address
            dst_virtual_address = atoi(dst_operand);

            //passed in adress is virtual so set dest physical memory location to -1
            int dst_memory_location = -1;

            //determine the VPN based on the dst_virtual_address and VPN bits
            int vpn = dst_virtual_address >> off;

            for (int i = 0; i < 8; i++) {
                if (tlb[i].valid && tlb[i].process_id == current_process && tlb[i].vpn == (dst_virtual_address >> off)) {
                    // TLB hit
                    dst_memory_location = (tlb[i].pfn << off) | (dst_virtual_address & ((1 << off) - 1));
                    fprintf(output_file, "Current PID: %d. Translating. Lookup for VPN %d hit in TLB entry %d. PFN is %d\n", current_process, vpn, i, tlb[i].pfn);
                    if (strcmp(strategy, "LRU") == 0) {
                        tlb[i].timestamp = timestamp;
                    }
                    break;
                }
            }

            if (dst_memory_location == -1) {
                // TLB miss, perform page table lookup
                if (page_tables[current_process][vpn].valid) {
                    dst_memory_location = (page_tables[current_process][vpn].pfn << off) | (dst_virtual_address & ((1 << off) - 1));
                    fprintf(output_file, "Current PID: %d. Translating. Lookup for VPN %d miss in TLB. PFN is %d\n", current_process, vpn, page_tables[current_process][vpn].pfn);

                } else {
                    //handle page table miss
                    fprintf(output_file, "Current PID: %d. Error: Page table miss for VPN %d\n", current_process, vpn);
                    return -1;
                }
            }

            //check if the source operand is a register or an immediate
            if (src_operand[0] == '#') {
                src_value = atoi(&src_operand[1]);
                fprintf(output_file, "Current PID: %d. Stored immediate %d into location %d\n", current_process, src_value, dst_virtual_address);
            } else {
                int src_register = atoi(&src_operand[1]);

                //get the value from the source register
                src_value = registers[src_register - 1];

                fprintf(output_file, "Current PID: %d. Stored value of register %s (%d) into location %d\n", current_process, src_operand, src_value, dst_virtual_address);
            }

            //check if the memory location is valid
            if (dst_memory_location >= 0 && dst_memory_location < num_frames) {
                //store the value into the memory location
                physical_memory[dst_memory_location] = src_value;
            } else {
                //handle invalid memory location
                fprintf(output_file, "Current PID: %d. Error: invalid memory location %d\n", current_process, dst_memory_location);
                return -1;
            }

        } else if (strcmp(tokens[0], "load") == 0) {
            if (tokens[1] == NULL || tokens[2] == NULL) {
                fprintf(output_file, "Current PID: %d. Error: Invalid load instruction format\n", current_process);
                return -1;
            }

            //check that memory is initialized
            if (!memory_initialized){
                fprintf(output_file, "Current PID: %d. Error: attempt to execute instruction before define\n", current_process);
                return -1;
            }

            char* dst_register = tokens[1];
            char* src_operand = tokens[2];
            int reg; //index for registers array

            if (strcmp(dst_register, "r1") == 0) {
                reg = 0;
            } else if (strcmp(dst_register, "r2") == 0){
                reg = 1;
            } else {
                //handle invalid register operand
                fprintf(output_file, "Current PID: %d. Error: invalid register operand %s\n", current_process, dst_register);
                return -1;
            }

            if (src_operand[0] == '#') {
                //if operand is an immediate
                int immediate_value = atoi(&src_operand[1]);
                registers[reg] = immediate_value;
                fprintf(output_file, "Current PID: %d. Loaded immediate %d into register %s\n", current_process, registers[reg], dst_register);
            } else {
                //if operand is a memory location 
                int src_virtual_address = atoi(src_operand);
                int src_memory_location = -1;

                int vpn = src_virtual_address >> off;

                for (int i = 0; i < 8; i++) {
                    if (tlb[i].valid && tlb[i].process_id == current_process && tlb[i].vpn == vpn) {
                        // TLB hit
                        src_memory_location = (tlb[i].pfn << off) | (src_virtual_address & ((1 << off) - 1));
                        fprintf(output_file, "Current PID: %d. Translating. Lookup for VPN %d hit in TLB entry %d. PFN is %d\n", current_process, vpn, i, tlb[i].pfn);
                        if (strcmp(strategy, "LRU") == 0) {
                            tlb[i].timestamp = timestamp;
                        }
                        break;
                    }
                }

                if (src_memory_location == -1) {
                    //TLB miss, perform page table lookup
                    if (page_tables[current_process][vpn].valid) {
                        src_memory_location = (page_tables[current_process][vpn].pfn << off) | (src_virtual_address & ((1 << off) - 1));
                        fprintf(output_file, "Current PID: %d. Translating. Lookup for VPN %d miss in TLB. PFN is %d\n", current_process, vpn, page_tables[current_process][vpn].pfn);
                    } else {
                        //handle page table miss
                        fprintf(output_file, "Current PID: %d. Translating. Lookup for VPN %d caused a TLB miss\n", current_process, vpn);
                        fprintf(output_file, "Current PID: %d. Translating. Translation for VPN %d not found in page table\n", current_process, vpn);
                        return -1;
                    }
                }

                if (src_memory_location >= 0 && src_memory_location < num_frames) {
                    //load the value from the memory location into the destination register
                    registers[reg] = physical_memory[src_memory_location];
                    fprintf(output_file, "Current PID: %d. Loaded value of location %d (%d) into register %s\n", current_process, src_virtual_address, registers[reg], dst_register);
                } else {
                    //handle invalid memory location
                    fprintf(output_file, "Current PID: %d. Error: invalid memory location %d\n", current_process, src_memory_location);
                    return -1;
                }
                
            }
        } else if (strcmp(tokens[0], "add") == 0){

            int result = registers[0] + registers[1];

            //output the result
            fprintf(output_file, "Current PID: %d. Added contents of registers r1 (%d) and r2 (%d). Result: %d\n", current_process, registers[0], registers[1], result);

            //store the result in register r1
            registers[0] = result;
        } else if (strcmp(tokens[0], "rinspect") == 0){
            char* reg_to_inspect = tokens[1];
            int reg;

            if (strcmp(reg_to_inspect, "r1") == 0) {
                reg = 0;
            } else if (strcmp(reg_to_inspect, "r2") == 0) {
                reg = 1;
            } else {
                fprintf(output_file, "Current PID: %d. Error: Invalid register %s\n", current_process, reg_to_inspect);
                return -1;
            }

            //output the content of the register
            fprintf(output_file, "Current PID: %d. Inspected register %s. Content: %u\n", current_process, reg_to_inspect, registers[reg]);
        } else if (strcmp(tokens[0], "pinspect") == 0){
            int vpn = atoi(tokens[1]);

            int valid = page_tables[current_process][vpn].valid;
            int pfn = valid ? page_tables[current_process][vpn].pfn : 0;

            fprintf(output_file, "Current PID: %d. Inspected page table entry %d. Physical frame number: %d. Valid: %d\n", current_process, vpn, pfn, valid);
        } else if (strcmp(tokens[0], "linspect") == 0) {
            int pl = atoi(tokens[1]);

            unsigned int value = physical_memory[pl];
            fprintf(output_file, "Current PID: %d. Inspected physical location %d. Value: %u\n", current_process, pl, value);

         } else if (strcmp(tokens[0], "tinspect") == 0){
            int tlb_number = atoi(tokens[1]);

            struct TLBEntry tlb_entry = tlb[tlb_number];

            fprintf(output_file, "Current PID: %d. Inspected TLB entry %d. VPN: %d. PFN: %d. Valid: %d. PID: %d. Timestamp: %d\n",
                current_process, tlb_number, tlb_entry.vpn, tlb_entry.pfn, tlb_entry.valid, tlb_entry.process_id, tlb_entry.timestamp);
         }


        //deallocate tokens
        for (int i = 0; tokens[i] != NULL; i++)
            free(tokens[i]);
        free(tokens);
    }

    // Free each of the page table arrays
    for (int i = 0; i < 4; i++) {
        free(page_tables[i]);
    }

    // Now free the array of pointers
    free(page_tables);

    //free physical memory
    free(physical_memory);

    //close input and output files
    fclose(input_file);
    fclose(output_file);

    return 0;

    
}