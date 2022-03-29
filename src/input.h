// input.h
// Matthew Jakeman (mjak923)

#pragma once

#include "common.h"
#include "invocation.h"

char *get_input ();
invocation_t *parse_input (char *input);
