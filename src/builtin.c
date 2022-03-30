// builtin.c
// Matthew Jakeman (mjak923)

#include "builtin.h"

/**
 * builtin_run_chdir:
 *
 * Builtin command for changing the current working directory.
 *
 * @tokens: String array of tokens
 * @home_dir: Absolute path to home directory
 *
 * Returns: Whether the builtin was handled successfully
 *
 */
bool
builtin_run_chdir (char **tokens,
                   char  *home_dir)
{
    char *arg;

    arg = tokens[1];

    if (!arg) {
        chdir (home_dir);
        return TRUE;
    }

    if (chdir (arg) == -1) {
        printf ("No such directory '%s'\n", arg);
    }

    return TRUE;
}

bool
builtin_run_jobs (job_dir_t *jobs)
{
    job_dir_print_all (jobs);
    return TRUE;
}

bool
builtin_run_fg (char      **tokens,
                job_dir_t  *jobs)
{
    char *arg;
    int index;

    arg = tokens[1];

    if (arg) {
        index = atoi (arg);
        job_dir_run_as_foreground (jobs, index);
        return TRUE;
    }

    job_dir_run_as_foreground (jobs, -1);
    return TRUE;
}

bool
builtin_run_bg (char      **tokens,
                job_dir_t  *jobs)
{
    char *arg;
    int index;

    arg = tokens[1];

    if (arg) {
        index = atoi (arg);
        job_dir_run_as_background (jobs, index);
        return TRUE;
    }

    job_dir_run_as_background (jobs, -1);
    return TRUE;
}

bool
builtin_run_kill (char      **tokens,
                  job_dir_t  *jobs)
{
    char *arg;
    int index;

    arg = tokens[1];

    if (arg) {
        index = atoi (arg);
        job_dir_kill (jobs, index);
        return TRUE;
    }

    job_dir_kill (jobs, -1);
    return TRUE;
}

/**
 * builtin_run_history:
 *
 * Builtin command for viewing and replaying history.
 *
 * @tokens: String array of tokens
 * @state: Opaque shell state structure
 * @history: History object
 *
 * Returns: Whether the builtin should inhibit normal execution
 *
 */
bool
builtin_run_history (char         **tokens,
                     invocation_t  *invocation,
                     history_t     *history)
{
    char *arg;

    arg = tokens[1];

    if (arg) {
        // handle replaying
        history_transform (history, &invocation);
        return FALSE;
    }

    history_print (history);
    return TRUE;
}
