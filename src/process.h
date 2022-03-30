// process.h
// Matthew Jakeman (mjak923)

#pragma once

#include "common.h"

#include "invocation.h"

enum process_state
{
    PROCESS_STATE_SLEEPING,
    PROCESS_STATE_RUNNABLE,
    PROCESS_STATE_STOPPED,
    PROCESS_STATE_IDLE,
    PROCESS_STATE_ZOMBIE,
    PROCESS_STATE_UNKNOWN
};

pid_t
process_run (invocation_t *invocation);

enum process_state
process_get_state (pid_t pid);
