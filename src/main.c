#include "common.h"

#include "input.h"
#include "history.h"
#include "builtin.h"
#include "invocation.h"
#include "process.h"

#include <libgen.h>
#include <sys/wait.h>

struct job_t;

typedef struct
{
    int n_jobs;
    int cur_id;
    struct job_t *jobs;
} job_dir_t;

typedef struct job_t
{
    int id;
    pid_t pid;
    struct job_t *next;
    bool dirty;
} job_t;

job_dir_t *
job_dir_new ()
{
    job_dir_t *self;
    self = malloc (sizeof (job_dir_t));
    return self;
}

void
job_dir_register_job (job_dir_t *self,
                      pid_t      pid)
{
    job_t *iter;
    job_t *job;

    job = malloc (sizeof (job_t));
    job->pid = pid;
    job->id = self->cur_id++;

    if (!self->jobs) {
        self->jobs = job;
        self->n_jobs = 1;
        return;
    }

    iter = self->jobs;
    while (iter != NULL) {
        if (iter->next == NULL) {
            iter->next = job;
            self->n_jobs++;
            return;
        }

        iter = iter->next;
    }
}

void
job_dir_unregister_job (job_dir_t *self,
                        job_t     *job)
{
    job_t *iter;

    if (!job) {
        return;
    }

    if (self->jobs == job) {
        self->jobs = job->next;

        free (job);
        if (--self->n_jobs == 0) {
            self->cur_id = 1;
        }
        return;
    }

    iter = self->jobs;
    while (iter != NULL) {

        if (iter->next == job) {
            job_t *after;
            after = iter->next->next;
            iter->next = after;

            free (job);
            if (--self->n_jobs == 0) {
                self->cur_id = 1;
            }
            return;
        }

        iter = iter->next;
    }
}

void
job_dir_iterate (job_dir_t *self)
{
    job_t *iter;

    // iterates over all jobs to see if any have finished
    for (iter = self->jobs;
         iter != NULL;
         iter = iter->next) {
        if (waitpid (iter->pid, NULL, WNOHANG)) {
            // job state has changed
            // todo: check if this is termination (or stopped/started/etc)

            // flag job for removal
            iter->dirty = TRUE;
        }
    }
}

void
job_dir_print_all (job_dir_t *self)
{
    job_t *iter;

    for (iter = self->jobs;
         iter != NULL;
         iter = iter->next) {
        printf ("job id %d / pid %d\n", iter->id, iter->pid);
    }
}

struct state_t
{
    char *home_dir;
    history_t *history;
    job_dir_t *jobs;
};

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

    if ((strcmp (tokens[0], "h") == 0) ||
        (strcmp (tokens[0], "history") == 0)) {
        // TODO: Handle equivalent history index (i.e. on the fourth command, issuing 'h 4')
        // return builtin_run_history (tokens, state, state->history);
    }

    free (tokens);

    return FALSE;
}

void
dispatch (state_t      *state,
          invocation_t *invocation)
{
    pid_t pid;

    // history_push (state->history, tokens);

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
        job_dir_register_job (state->jobs, pid);
    }
    else {
        waitpid (pid, NULL, 0);
    }
}

int main ()
{
    printf ("mAsh! Matthew's Shell\n");

    bool running;
    state_t state;
    state.history = history_new ();
    state.jobs = job_dir_new ();
    state.home_dir = malloc (sizeof (char) * BUFFER_SIZE);
    getcwd (state.home_dir, BUFFER_SIZE);

    running = TRUE;

    while (running)
    {
        invocation_t *invocation;
        char *input;

        // check jobs
        job_dir_print_all (state.jobs);

        print_prompt ();
        input = get_input ();
        invocation = parse_input (input);

        dispatch (&state, invocation);

        // invocation_free (invocation);
    }
}
