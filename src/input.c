// input.c
// Matthew Jakeman (mjak923)

#include "input.h"
#include "invocation.h"

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

invocation_t *
parse_input (char *input)
{
    int index;
    int cmd_size;
    char **tokens;
    char *cur_token;
    invocation_t *invocation;

    // Input is a string ending with \n
    if (!input) {
        return NULL;
    }

    // Allocate invocation (and therefore tokens)
    invocation = invocation_new ();
    tokens = invocation->tokens;
    cmd_size = 0;
    index = 0;

    // Get command token
    cur_token = strtok (input, " \t");

    // Get argument tokens
    while (cur_token) {
        // Handle pipes
        if (strcmp (cur_token, "|") == 0) {
            invocation_push_command (invocation, index - cmd_size, cmd_size);
            cmd_size = 0;
        }
        else {
            cmd_size++;
        }

        tokens[index++] = cur_token;
        cur_token = strtok (NULL, " \t");

        if (index > TOKEN_ARRAY_SIZE) {
            fprintf (stderr, "Exceeded maximum number of arguments (%d)\n", TOKEN_ARRAY_SIZE);
            break;
        }
    }

    // check if last token is '&'
    if (index > 0 && strcmp (tokens[index-1], "&") == 0) {
        invocation->is_job = TRUE;
    }

    index = index - cmd_size;

    // exclude '&' token if job
    cmd_size = invocation->is_job
        ? cmd_size-1
        : cmd_size;

    // push final command
    if (cmd_size > 0) {
        invocation_push_command (invocation, index, cmd_size);
    }

    return invocation;
}
