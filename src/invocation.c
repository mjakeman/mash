// invocation.c
// Matthew Jakeman (mjak923)

#include "invocation.h"

invocation_t *
invocation_new ()
{
    invocation_t *result = calloc (1, sizeof (invocation_t));
    result->tokens = calloc (1, sizeof (char *) * TOKEN_ARRAY_SIZE);
    return result;
}

char **
invocation_command_get_tokens (invocation_t *self,
                               command_t    *command)
{
    // caller must free!

    char **tokens_cpy;
    int n_tokens;
    int index;

    n_tokens = command->n_tokens;
    index = command->index;

    // return a null terminated array containing
    // only the tokens for this command (needed for execvp)
    tokens_cpy = calloc (1, sizeof (char *) * (n_tokens + 1));
    memcpy (tokens_cpy, &self->tokens[index], n_tokens * sizeof (char *));
    tokens_cpy [n_tokens] = NULL;

    return tokens_cpy;
}

void
invocation_push_command (invocation_t *self,
                         int           index,
                         int           n_tokens)
{
    command_t *command;
    command_t *iter;

    command = calloc (1, sizeof (command_t));
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

invocation_t *
invocation_copy (invocation_t *self)
{
    invocation_t *new;
    size_t token_size;
    command_t *old_iter;
    command_t *prev;

    // deep copy
    new = calloc (1, sizeof (invocation_t));
    new->is_job = self->is_job;
    new->n_commands = self->is_job;

    token_size = sizeof (char *) * TOKEN_ARRAY_SIZE;
    new->tokens = calloc (1, token_size);
    memcpy (new->tokens, self->tokens, token_size);

    prev = NULL;

    // deep copy commands
    for (old_iter = self->commands;
         old_iter != NULL;
         old_iter = old_iter->next) {

        command_t *command;
        command = calloc (1, sizeof (command_t));
        command->n_tokens = old_iter->n_tokens;
        command->index = old_iter->index;
        command->next = NULL;

        if (prev) {
            prev->next = command;
        }
        else {
            new->commands = command;
        }

        prev = command;
    }

    return new;
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
