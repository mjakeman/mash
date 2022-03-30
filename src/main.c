#include "common.h"

#include "input.h"
#include "history.h"
#include "builtin.h"
#include "invocation.h"
#include "process.h"
#include "job.h"

#include <libgen.h>
#include <sys/wait.h>

struct state_t
{
    char *home_dir;
    history_t *history;
    job_dir_t *jobs;
};

static state_t state;

void print_prompt ()
{
    char *cur_dir;
    char cwd[BUFFER_SIZE];

    cur_dir = "error";
    if (getcwd (cwd, BUFFER_SIZE))
        cur_dir = basename (cwd);

    printf (ANSI_COLOR_RED);
    printf ("[%s]# ", cur_dir);
    printf (ANSI_COLOR_RESET);
}

bool
handle_builtin (state_t      *state,
                invocation_t *invocation)
{
    command_t *first_command;
    char **tokens;

    first_command = invocation->commands;

    if (!first_command || !(first_command->n_tokens))
        return FALSE;

    // get tokens as null terminated array
    tokens = invocation_command_get_tokens (invocation, first_command);

    if (strcmp (tokens[0], "exit") == 0) {
        exit (EXIT_SUCCESS);
    }

    if (strcmp (tokens[0], "cd") == 0) {
        return builtin_run_chdir (tokens, state->home_dir);
    }

    if (strcmp (tokens[0], "jobs") == 0) {
        return builtin_run_jobs (state->jobs);
    }

    if ((strcmp (tokens[0], "h") == 0) ||
        (strcmp (tokens[0], "history") == 0)) {
        // TODO: Handle equivalent history index (i.e. on the fourth command, issuing 'h 4')
        return builtin_run_history (tokens, state, state->history);
    }

    if (strcmp (tokens[0], "fg") == 0) {
        return builtin_run_fg (tokens, state->jobs);
    }

    if (strcmp (tokens[0], "bg") == 0) {
        return builtin_run_bg (tokens, state->jobs);
    }

    if (strcmp (tokens[0], "kill") == 0) {
        return builtin_run_kill (tokens, state->jobs);
    }

    free (tokens);

    return FALSE;
}

void
dispatch (state_t      *state,
          invocation_t *invocation)
{
    pid_t pid;
    bool handled;

    // check if built-in and return, otherwise proceed as normal
    //  - from piazza @23: ignore the use of built-in commands in pipelines
    //  - from piazza @30: do not need to run built-in commands as jobs
    handled = handle_builtin (state, invocation);

    // we need to push after handling built-ins to support replaying
    history_push (state->history, invocation);

    if (handled) {
        return;
    }

    // execute process, including pipelines
    pid = process_run (invocation);

    if (pid < 0) {
        return;
    }

    if (invocation->is_job) {
        // push new job to directory
        job_dir_register_job (state->jobs, invocation, pid);
    }
    else {
        job_dir_new_in_foreground (state->jobs, invocation, pid);
    }
}

void
handler ()
{
    // Register job
    job_dir_suspend_foreground (state.jobs);
    signal (SIGTSTP, handler);
}

int main ()
{
    printf ("mAsh! Matthew's Shell\n");

    bool running;
    state.history = history_new ();
    state.jobs = job_dir_new ();
    state.home_dir = calloc (1, sizeof (char) * BUFFER_SIZE);
    getcwd (state.home_dir, BUFFER_SIZE);

    signal (SIGTSTP, handler);

    running = TRUE;

    while (running)
    {
        invocation_t *invocation;
        char *input;

        // check jobs
        job_dir_iterate (state.jobs);
        job_dir_flush (state.jobs);

        print_prompt ();
        input = get_input ();
        invocation = parse_input (input);

        dispatch (&state, invocation);

        invocation_free (invocation);
        invocation = NULL;
    }

    // todo: kill all job processes
}
