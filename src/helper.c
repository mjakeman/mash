// helper.c
// Matthew Jakeman (mjak923)

#include "common.h"

// Helper method for printing out tokens
// as a single string robustly.
char *
tokens_to_string (char **tokens)
{
    int buffer_size;
    char *output;
    char *token;
    int index;
    int cur_length;

    if (!tokens || !tokens[0])
        return NULL;

    buffer_size = BUFFER_SIZE;
    output = calloc (1, sizeof (char) * buffer_size);
    output[0] = '\0';

    index = 0;
    token = tokens[index];
    cur_length = 0;
    while (token) {
        int result;
        int add_length;

        add_length = strlen (token);
        result = sprintf (output, "%s %s", output, token);

        if (result < 0) {
            int delta;

            // Buffer is too small
            // Increase buffer by the larger of BUFFER_SIZE or add_length
            delta = BUFFER_SIZE > add_length
                ? BUFFER_SIZE
                : add_length;

            // Reallocate output buffer
            buffer_size += delta;
            output = realloc (output, buffer_size);

            // Try again
            result = sprintf (output, "%s %s", output, token);

            if (result < 0) {
                fprintf (stderr, "Error appending token - output may be corrupt\n");
                return output;
            }
        }

        cur_length += add_length;
        token = tokens[++index];
    }

    return output;
}
