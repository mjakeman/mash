// invocation.h
// Matthew Jakeman (mjak923)

#pragma once

#include "common.h"

struct command_t
{
    int index;
    int n_tokens;
    struct command_t *next;
};

struct invocation_t
{
    command_t *commands;
    int n_commands;
    char **tokens;
    bool is_job;
};

void
invocation_free (invocation_t *self);

void
invocation_push_command (invocation_t *self,
                         int           index,
                         int           n_tokens);

char **
invocation_command_get_tokens (invocation_t *self,
                               command_t    *command);

invocation_t *
invocation_copy (invocation_t *self);

invocation_t *
invocation_new ();
