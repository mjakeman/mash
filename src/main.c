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

void
print_if_file (invocation_t *invocation)
{
    if (!isatty (STDIN_FILENO)) {
        char *string;
        string = tokens_to_string (invocation->tokens);
        printf ("%s\n", string);
        free (string);
    }
}

void
print_prompt ()
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
        RUNNING = FALSE;
        return TRUE;
    }

    if (strcmp (tokens[0], "cd") == 0) {
        return builtin_run_chdir (tokens, state->home_dir);
    }

    if (strcmp (tokens[0], "jobs") == 0) {
        return builtin_run_jobs (state->jobs);
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
replay_history (state_t *state,
                char    *arg)
{
    int index;
    int min;
    int max;
    invocation_t *replay;
    invocation_t *copy;

    history_get_range (state->history, &min, &max);

    index = atoi (arg);

    if (index == 0)
        goto error;

    if (index < min || index > max)
        goto error;

    // get the invocation to replay
    replay = history_get_invocation (state->history, index);

    // perform a deep copy and execute
    copy = invocation_copy (replay);
    dispatch (state, copy);
    invocation_free (copy);

    return;

error:
    printf ("Argument must be a number between %d and %d\n", min, max);
}

bool
handle_history (state_t      *state,
                invocation_t *invocation)
{
    // returns
    //  - TRUE if the command has been handled
    //  - FALSE if execution should continue

    command_t *first_command;
    char **tokens;
    char *arg;

    first_command = invocation->commands;

    if (!first_command || !(first_command->n_tokens))
        return FALSE;

    // get tokens as null terminated array
    tokens = invocation_command_get_tokens (invocation, first_command);

    // return if we are not the history command
    if ((strcmp (tokens[0], "h") != 0) &&
        (strcmp (tokens[0], "history") != 0)) {
        return FALSE;
    }

    arg = tokens[1];

    if (arg) {
        replay_history (state, arg);
    }
    else {
        // add this command before printing
        print_if_file (invocation);
        history_push (state->history, invocation);
        history_print (state->history);
    }

    return TRUE;
}

void
dispatch (state_t      *state,
          invocation_t *invocation)
{
    pid_t pid;

    if (!invocation)
        return;

    // return if no command
    if (invocation->n_commands == 0)
        return;

    // special case the history command as history needs to
    // be aware of itself and replay an earlier invocation if
    // necessary
    if (handle_history (state, invocation)) {
        return;
    }

    // Print the invocation input is being redirected from a file
    print_if_file (invocation);

    // Add this invocation to history
    history_push (state->history, invocation);

    // check if built-in and return, otherwise proceed as normal
    //  - from piazza @23: ignore the use of built-in commands in pipelines
    //  - from piazza @30: do not need to run built-in commands as jobs
    if (handle_builtin (state, invocation)) {
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
suspend ()
{
    // Register job
    job_dir_suspend_foreground (state.jobs);
    signal (SIGTSTP, suspend);
}

void
shutdown ()
{
    RUNNING = FALSE;
}

int main ()
{
    printf ("mAsh! Matthew's Shell\n");

    state.history = history_new ();
    state.jobs = job_dir_new ();
    state.home_dir = calloc (1, sizeof (char) * BUFFER_SIZE);
    getcwd (state.home_dir, BUFFER_SIZE);

    signal (SIGTSTP, suspend);
    signal (SIGINT, shutdown);

    RUNNING = TRUE;

    while (RUNNING)
    {
        invocation_t *invocation;
        char *input;

        // check jobs
        job_dir_iterate (state.jobs);
        job_dir_flush (state.jobs);

        print_prompt ();

        input = get_input ();

        // Check for EOF if using file input
        if (!isatty (STDIN_FILENO) && !input) {
            RUNNING = FALSE;
            break;
        }

        invocation = parse_input (input);
        dispatch (&state, invocation);

        invocation_free (invocation);
        invocation = NULL;
    }

    // Kill all jobs
    printf ("Goodbye!\n");
    job_dir_kill_all (state.jobs);
}
