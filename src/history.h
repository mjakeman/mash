// history.h
// Matthew Jakeman (mjak923)

#pragma once

#include "common.h"

struct history_t;
typedef struct history_t history_t;

history_t *history_new ();

void history_push (history_t  *self,
                   char      **tokens);

void history_print (history_t *self);

void
history_get_range (history_t *history,
                   int       *min,
                   int       *max);

char **
history_get_tokens (history_t *self,
                    int        id);
