#include "util.h"

int* read_next_line(FILE* file) {
    if (file == NULL) {
        fprintf(stderr, "File pointer is NULL\n");
        return NULL;
    }

    char* line = NULL;
    size_t len = 0;
    char* token;

    // Read the next line from the file
    if (getline(&line, &len, file) == -1) {
        free(line); // Free allocated memory
        return NULL; // No more lines to read
    }

    // Tokenize the line based on commas
    int count = 0;
    int* integers = NULL;

    token = strtok(line, ",");
    while (token != NULL) {
        int value = atoi(token);
        integers = (int*)realloc(integers, (count + 1) * sizeof(int));
        if (integers == NULL) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        integers[count] = value;
        count++;
        token = strtok(NULL, ",");
    }

    // Allocate memory for the integer array and store the count as the first element
    int* result = (int*)malloc((count + 1) * sizeof(int));
    if (result == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    result[0] = count;
    memcpy(&result[1], integers, count * sizeof(int));

    // Free allocated memory
    free(line);
    free(integers);

    return result;
}


float compute_average(int* line) {
    // TODO: Compute the average of the integers in the vector
    // Recall that the first element of the vector is the number of integers

    float average = 0;
    for(int i=1; i<=line[0]; i++){
        average += line[i];
    }

    return average/line[0];

}


float compute_stdev(int* line) {
    // TODO: Compute the standard deviation of the integers in the vector
    // Recall that the first element of the vector is the number of integers
    float average = compute_average(line);
    float stdev = 0;
    for(int i=1; i<=line[0]; i++){
        stdev += pow(line[i]-average, 2);
    }
    stdev /= line[0];
    return sqrt(stdev); // might be stdev/line[0]-1 instead
}