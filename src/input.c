// input.c
// Matthew Jakeman (mjak923)

#include "input.h"

#define TOKEN_ARRAY_SIZE 100

invocation_t *
invocation_new ()
{
    invocation_t *result = malloc (sizeof (invocation_t));
    result->tokens = malloc (sizeof (char *) * TOKEN_ARRAY_SIZE);
    return result;
}

void
invocation_push_command (invocation_t *self,
                         int           index,
                         int           n_tokens)
{
    command_t *command;
    command_t *iter;

    command = malloc (sizeof (command_t));
    command->index = index;
    command->n_tokens = n_tokens;

    if (!self->commands) {
        self->commands = command;
        self->n_commands = 1;
        return;
    }

    iter = self->commands;
    while (iter != NULL) {
        if (iter->next == NULL) {
            iter->next = command;
            self->n_commands++;
            return;
        }

        iter = iter->next;
    }
}

void
invocation_free (invocation_t *self)
{
    command_t *iter;
    command_t *to_free;

    iter = self->commands;

    while (iter != NULL) {
        to_free = iter;
        iter = iter->next;

        free (to_free);
    }

    free (self->tokens);
    free (self);
}

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

    // only push if last token is not pipe
    if (cmd_size != 0) {
        invocation_push_command (invocation, index - cmd_size, cmd_size);
    }

    return invocation;
}
