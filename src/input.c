// input.c
// Matthew Jakeman (mjak923)

#include "input.h"

#define TOKEN_ARRAY_SIZE 100

char *
get_input ()
{
    char *buffer = NULL;
    size_t buffer_size = 0;
    int length;

    // Read in input up to a newline '\n' character
    length = getline (&buffer, &buffer_size, stdin);

    if (length == -1) {
        fprintf (stderr, "Unable to read line\n");
        return NULL;
    }

    // getline includes the newline character but no null
    // termination. Replace the '\n' with '\0'.
    if ((length > 0) && (buffer[length - 1] == '\n')) {
        buffer[length - 1] = '\0';
    }

    return buffer;
}

char **
parse_input (char  *input,
             int   *n_tokens)
{
    int index;
    char **tokens;
    char *cur_token;

    // Input is a string ending with \n
    if (!input) {
        *n_tokens = 0;
        return NULL;
    }

    // Allocate tokens
    tokens = malloc (sizeof (char *) * TOKEN_ARRAY_SIZE);
    index = 0;

    // Get command token
    cur_token = strtok (input, " \t");

    // Get argument tokens
    while (cur_token) {
        tokens[index++] = cur_token;
        cur_token = strtok (NULL, " \t");

        if (index > TOKEN_ARRAY_SIZE) {
            fprintf (stderr, "Exceeded maximum number of arguments (%d)\n", TOKEN_ARRAY_SIZE);
            break;
        }
    }

    *n_tokens = index+1;
    return tokens;
}
