#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

FILE *output = NULL;

/*------------------------------- Prefix sum start -------------------------------*/
typedef struct {
    int* array;
    int count;
    int start;
    int stride;
    pthread_barrier_t *barrier;
} threadData;

void* prefix_sum(void *arg) {
    threadData *data = (threadData *)arg; 
    int *array = data->array;
    int count = data->count;
    int start = data->start;
    int stride = data->stride;
    pthread_barrier_t* barrier = data->barrier;

    // Initialize first array
    int *midArray = (int *)malloc(count * sizeof(int));
    for (int i = 0; i < count; i++) {
        midArray[i] = array[i];
    }
    
    // Perform calculations
    for (int i = 0; i < log2(count); i++) {
        for (int j = start; j < count; j += stride) {
            if (j - (int)pow(2, i) >= 0) {
                midArray[j] = array[j] + array[j - (int)pow(2, i)];
            }
        }
        pthread_barrier_wait(barrier);
        for (int j = start; j < count; j += stride) {
            array[j] = midArray[j];
        }
        pthread_barrier_wait(barrier);
    }
    free(midArray);
    pthread_exit(NULL);
}
/*------------------------------- Prefix sum end -------------------------------*/

// Parse prefix_arr vector of integers from prefix_arr file, one per line.
// Return the number of integers parsed.
int parse_ints(FILE *file, int **ints) {
    int value;
    int count = 0; // Number of ints stored.

    *ints = NULL;

    while (fscanf(file, "%d", &value) == 1){
        int *temp = realloc(*ints, (count + 1) * sizeof(int));
        if (!temp){
            perror("Memory Allocation Failed");
            free(*ints);
            return -1;
        }
        *ints = temp;
        (*ints)[count++] = value;
    }

    return count;
}

// Write prefix_arr vector of integers to prefix_arr file, one per line.
void write_ints(FILE *file, const int *ints, int count) {
    if (file == NULL || ints == NULL){
        fprintf(stderr, "Invalid File or *ints.\n");
        return;
    }

    for (int i = 0; i < count; i++){
        fprintf(file, "%d\n", ints[i]);
    }
}

// Return the result of prefix_arr sequential prefix scan of the given vector of integers.
int *SEQ(int *ints, int count) {
    int *prefix_arr = (int *)malloc(count * sizeof(int));
    prefix_arr[0] = ints[0];
    for (int i = 1; i < count; i++){
        prefix_arr[i] = ints[i] + prefix_arr[i - 1];
    }
    return prefix_arr;
}

// Return the result of Hillis/Steele, but with each pass executed sequentially
int* HSS(int *ints, int count) {
    // Initialize arrays
    int** arrays = (int**)malloc(log2(count)* sizeof(int*));

    // Initialize First Line
    arrays[0] = (int *)malloc(count * sizeof(int));
    arrays[0][0] = ints[0];
    for (int i = 1; i < count; i++){
        arrays[0][i] = ints[i] + ints[i - 1];;
    }

    for (int i = 1; i < log2(count); i++){
        // initialize row i
        arrays[i] = (int *)malloc(count * sizeof(int));

        for (int j = 0; j < pow(2, i); j++){
            arrays[i][j] = arrays[i - 1][j];
        }

        for(int j = pow(2, i); j < count; j++){
            arrays[i][j] = arrays[i - 1][j] + arrays[i - 1][j - (int)pow(2, i)];
        }
    }

    return arrays[(int)log2(count) - 1];
}

// Return the result of Hillis/Steele, parallelized using pthread
int* HSP(int *ints, int count, int numthreads) {
    // Initiallize Return Array
    int* array = (int*)malloc(count * sizeof(int));
    for(int i = 0; i < count; i++){
        array[i] = ints[i];
    }
    
    pthread_t threads[numthreads];
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, numthreads);
    threadData data[numthreads];

    // Initialize the threads
    for (int i = 0; i < numthreads; i++) {
        data[i].array = array;
        data[i].start = i;
        data[i].stride = numthreads;
        data[i].count = count;
        data[i].barrier = &barrier;

        pthread_create(&threads[i], NULL, prefix_sum, (void *)&data[i]);
    }

    // join the threads
    for (int i = 0; i < numthreads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);
    return array;
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
    int count;
    count = parse_ints(input, &ints);

    int *result;
    if (strcmp(mode, "SEQ") == 0) {
        result = SEQ(ints, count);
    }
    else if (strcmp(mode, "HSS") == 0) {
        result = HSS(ints, count);
    }
    else if (strcmp(mode, "HSP") == 0) {
        result = HSP(ints, count, num_threads);
    }
    else {
        printf("Unknown mode: %s\n", mode);
        return 1;
    }

    write_ints(output, result, count);
    fclose(input);
    fclose(output);
    free(ints);
    return 0;
}