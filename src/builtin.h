// builtin.h
// Matthew Jakeman (mjak923)

#include "common.h"
#include "history.h"

bool builtin_run_chdir (char **tokens,
                        char  *home_dir);

bool builtin_run_history (char      **tokens,
                          state_t    *state,
                          history_t  *history);
