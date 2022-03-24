#pragma once

#include "common.h"

struct history_t;
typedef struct history_t history_t;

history_t *history_new ();

void history_push (history_t  *self,
                   char      **tokens);

void history_print (history_t *self);
