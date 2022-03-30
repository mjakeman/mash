// history.h
// Matthew Jakeman (mjak923)

#pragma once

#include "common.h"
#include "invocation.h"

struct history_t;
typedef struct history_t history_t;

history_t *history_new ();

void history_push (history_t    *self,
                   invocation_t *invocation);

void history_print (history_t *self);

void
history_get_range (history_t *history,
                   int       *min,
                   int       *max);

invocation_t *
history_get_invocation (history_t *self,
                        int        id);

void
history_transform (history_t     *self,
                   invocation_t **command);
