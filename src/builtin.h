// builtin.h
// Matthew Jakeman (mjak923)

#include "common.h"
#include "history.h"
#include "job.h"

bool builtin_run_chdir (char **tokens,
                        char  *home_dir);

bool builtin_run_history (char      **tokens,
                          state_t    *state,
                          history_t  *history);

bool builtin_run_jobs (job_dir_t *jobs);

bool builtin_run_fg (char      **tokens,
                     job_dir_t  *jobs);

bool builtin_run_bg (char      **tokens,
                     job_dir_t  *jobs);

bool builtin_run_kill (char      **tokens,
                       job_dir_t  *jobs);
