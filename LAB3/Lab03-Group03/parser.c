#include "parser.h"

//Command to trim whitespace and ASCII control characters from buffer
//[Input] char* inputbuffer - input string to trim
//[Input] size_t bufferlen - size of input and output string buffers
//[Output] char* outputbuffer - output string after trimming 
//[Return] size_t - size of output string after trimming
char* trimstring(char* outputbuffer, const char* inputbuffer, size_t bufferlen)
{   
    memcpy(outputbuffer, inputbuffer, bufferlen*sizeof(char));
    for(size_t ii = strlen(outputbuffer)-1; ii >=0; ii--){
        if(outputbuffer[ii] < '!') //In ASCII '!' is the first printable (non-control) character
        {
            outputbuffer[ii] = 0;
        }else{
            break;
        }    
    }

    size_t i = 0;
    while (i < (strlen(outputbuffer) - 1)) {
        if(outputbuffer[i] >= '!') //In ASCII '!' is the first printable (non-control) character
        {
            break;
        }
        i++;
    }
    outputbuffer += (i*sizeof(char));
    return outputbuffer;
}

//Command to trim the input command to just be the first word
//[Input] char* inputbuffer - input string to trim
//[Input] size_t bufferlen - size of input and output string buffers
//[Output] char* outputbuffer - output string after trimming 
//[Return] size_t - size of output string after trimming
size_t firstword(char* outputbuffer, const char* inputbuffer, size_t bufferlen)

{
    //TO DO: Implement this function
    // Check for NULL pointers or empty input
    if (inputbuffer == NULL || outputbuffer == NULL || bufferlen == 0) {
        return 0;
    }

    size_t input_len = strlen(inputbuffer);
    size_t output_len = 0;

    // Iterate through the input string until a space is encountered
    for (size_t i = 0; i < input_len; i++) {
        if (inputbuffer[i] == ' ') {
            break; // Stop when a space is found
        }

        // Check if there's enough space in the output buffer
        if (output_len < bufferlen - 1) {
            outputbuffer[output_len++] = inputbuffer[i];
        } else {
            // Output buffer is not large enough, return 0
            return 0;
        }
    }

    // Null-terminate the output string
    outputbuffer[output_len] = '\0';
    return output_len;
}

// DONE - DO NOT TOUCH
//Command to test that string only contains valid ascii characters (non-control and not extended)
//[Input] char* inputbuffer - input string to test
//[Input] size_t bufferlen - size of input buffer
//[Return] bool - true if no invalid ASCII characters present
bool isvalidascii(const char* inputbuffer, size_t bufferlen)
{
    //TO DO: Correct this function so that the second test string fails
    size_t testlen = bufferlen;
    size_t stringlength = strlen(inputbuffer);
    if(strlen(inputbuffer) < bufferlen){
        testlen = stringlength;
    }

    bool isValid = true;
    for(size_t ii = 0; ii < testlen; ii++)
    {
        //In (lower) ASCII '~' is the last printable character and " " is the first printable character
        isValid &= ((unsigned char) inputbuffer[ii] <= '~' && (unsigned char) inputbuffer[ii]>=' '); 
    } // edited the inpute buffer[ii] to check if it is greater than " " and less than "~" then last ascii character
    return isValid;
}

//Command to find location of pipe character in input string
//[Input] char* inputbuffer - input string to test
//[Input] size_t bufferlen - size of input buffer
//[Return] int - location in the string of the pipe character, or -1 pipe character not found
int findpipe(const char* inputbuffer, size_t bufferlen){
    //TO DO: Implement this function
    size_t testlen = bufferlen;
    size_t stringlength= strlen(inputbuffer);
    if(strlen(inputbuffer)<bufferlen){
        testlen = stringlength;
    }

    for(size_t j=0; j<testlen;j++){
        if(inputbuffer[j]=='|'){
            return j;
        }

    }
    return -1;
}

//Command to test whether the input string ends with "&" and
//thus represents a command that should be run in background
//[Input] char* inputbuffer - input string to test
//[Input] size_t bufferlen - size of input buffer
//[Return] bool - true if string ends with "&"
#include <stdbool.h>
#include <string.h>

bool runinbackground(const char* inputbuffer, size_t bufferlen) {
    // TO DO implement this function 
    // Check for NULL pointer or empty input
    if (inputbuffer == NULL || bufferlen == 0) {
        return false;
    }

    size_t input_len = strlen(inputbuffer);

    // Check if the input string is at least one character long
    if (input_len == 0) {
        return false;
    }

    // Check if the last character of the input is '&'
    if (inputbuffer[input_len - 1] == '&') {
        return true;
    }
    return false;
}

char** split(const char* input, char sep, int* count) {
    int num = 0;
    int inside_quotes = 0; // Flag to track if we are inside quotes
    int i, wordStart = 0, word_i = 0;

    for (i = 0; input[i] != '\0'; i++) {
        if (input[i] == '"') {
            inside_quotes = !inside_quotes; // Toggle the inside_quotes flag
        } else if (input[i] == sep && !inside_quotes) {
            num++;
        }
    }
    num++;

    char** strings = (char**)malloc(num * sizeof(char*));
    *count = num;

    for (i = 0; input[i] != '\0'; i++) {
        if (input[i] == '"') {
            inside_quotes = !inside_quotes; // Toggle the inside_quotes flag
        } else if (input[i] == sep && !inside_quotes) {
            int wordlen = i - wordStart;
            strings[word_i] = (char*)malloc((wordlen + 1) * sizeof(char));
            strncpy(strings[word_i], input + wordStart, wordlen);
            strings[word_i][wordlen] = '\0';
            wordStart = i + 1;
            word_i++;
        }
    }

    int wordlen = strlen(input) - wordStart;
    strings[word_i] = (char*)malloc((wordlen + 1) * sizeof(char));
    strncpy(strings[word_i], input + wordStart, wordlen);
    strings[word_i][wordlen] = '\0';

    for(int i = 0; i < num; i++){
        strip(strings[i], '"');
    }

    return strings;
}

void strip(char* string, char stripper){
    int len = strlen(string);
    int j = 0;

    for(int i = 0; i < len; i++){
        if (string[i] != stripper){
            string[j] = string[i];
            j++;
        }
    }

    string[j] = '\0';
}