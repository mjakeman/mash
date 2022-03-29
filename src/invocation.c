// invocation.c
// Matthew Jakeman (mjak923)

#include "invocation.h"

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
