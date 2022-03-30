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

typedef bool (*job_func)(job_dir_t *, int index);

static bool
run_for_job (char      **tokens,
             job_dir_t  *jobs,
             job_func    func)
{
    char *arg;
    int index;

    arg = tokens[1];

    if (arg) {
        index = atoi (arg);

        if (index == 0)
            goto error;

        if (!func (jobs, index))
            goto error;
    }
    else {
        if (!func (jobs, -1))
            goto error2;
    }

    return TRUE;

error:
    printf ("Must provide a valid job number.\n");
    return TRUE;

error2:
    printf ("There are no jobs.\n");
    return TRUE;
}

bool
builtin_run_fg (char      **tokens,
                job_dir_t  *jobs)
{
    return run_for_job (tokens, jobs, job_dir_run_as_foreground);
}

bool
builtin_run_bg (char      **tokens,
                job_dir_t  *jobs)
{
    return run_for_job (tokens, jobs, job_dir_run_as_background);
}

bool
builtin_run_kill (char      **tokens,
                  job_dir_t  *jobs)
{
    return run_for_job (tokens, jobs, job_dir_kill);
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
