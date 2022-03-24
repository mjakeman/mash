// common.h
// Matthew Jakeman (mjak923)

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>

#define bool int
#define FALSE 0
#define TRUE 1

#define BUFFER_SIZE 500

// Colour Output
// This uses ANSI escape codes to colour the terminal output. On platforms
// where colour output is not available, this should simply have no effect.
// All credit to: https://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// Opaque state structure
struct state_t;
typedef struct state_t state_t;

// Dispatch method
void dispatch (state_t  *state,
               char    **tokens);
