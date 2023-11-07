#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Parse prefix_arr vector of integers from prefix_arr file, one per line.
// Return the number of integers parsed.
int parse_ints(FILE *file, int **ints) {
    if (file == NULL) {
        perror("Error Opening File");
        return 0;
    }
    
    int count = 0;
    int currentInt;
    while (fscanf(file, "%d", &currentInt) == 1) {      // fscanf keeps reading when return value is 1, goes to each line
        count++;
        *ints = (int*)realloc(*ints, count * sizeof(int));      // realloc dynamic array
        if (*ints == NULL) {
            printf("Memory allocation failed.\n");
            return 1;
        }
        *ints[count - 1] = currentInt;
    }
    
    return count;
}


// Write prefix_arr vector of integers to prefix_arr file, one per line.
void write_ints(FILE *file, int *ints, int size) {
    if (file == NULL) {
        perror("Error Opening File");
        return 0;
    }

    for (int i = 0; i < size; i++) {
        fprintf(file, "%d\n", ints[i]);
    }
}


// Return the result of prefix_arr sequential prefix scan of the given vector of integers.
int* SEQ(int *ints, int size) {
    int prefix_arr[size];
    
    prefix_arr[0]= ints[0];
    for(int i=1;i<size;i++){
        prefix_arr[i] = ints[i] + ints[i-1]; // for sequential we add like this
    }

    return &prefix_arr;
}


// Return the result of Hillis/Steele, but with each pass executed sequentially
int* HSS(int *ints, int size) {
    int prefix_arr[size];
    prefix_arr[0]= ints[0];
    for(int i = 1; i < size; i++){
        prefix_arr[i]= prefix_arr[i-1] + ints[i];
    }
    return &prefix_arr;
}


// Return the result of Hillis/Steele, parallelized using pthread
int* HSP(int *ints, int size, int numthreads) {

}


int main(int argc, char** argv) {

    if ( argc != 5 ) {
        printf("Usage: %s <mode> <#threads> <input file> <output file>\n", argv[0]);
        return 1;
    }

    char *mode = argv[1];
    int num_threads = atoi(argv[2]);
    FILE *input = fopen(argv[3], "r");
    FILE *output = fopen(argv[4], "w");

    int *ints;
    int size;
    size = parse_ints(input, &ints);

    int *result;
    if (strcmp(mode, "SEQ") == 0) {
        result = SEQ(ints, size);
    } else if (strcmp(mode, "HSS") == 0) {
        result = HSS(ints, size);
    } else if (strcmp(mode, "HSP") == 0) {
        result = HSP(ints, size, num_threads);
    } else {
        printf("Unknown mode: %s\n", mode);
        return 1;
    }

    write_ints(output, result, size);
    fclose(input);
    fclose(output);

    free(ints);

    return 0;
}
