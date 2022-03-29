// input.h
// Matthew Jakeman (mjak923)

#pragma once

#include "common.h"

typedef struct command_t command_t;
typedef struct invocation_t invocation_t;

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

char *get_input ();
invocation_t *parse_input (char *input);
