// job.c
// Matthew Jakeman (mjak923)

#include "job.h"

#include "process.h"

#include <sys/wait.h>

#define FOREGROUND_JOB_ID -1

struct job_dir_t
{
    int n_jobs;
    int cur_id;
    struct job_t *jobs;
    job_t *foreground;
};

struct job_t
{
    int id;
    pid_t pid;
    invocation_t *invocation;
    struct job_t *next;
    bool dirty;
};

job_dir_t *
job_dir_new ()
{
    job_dir_t *self;
    self = calloc (1, sizeof (job_dir_t));
    self->cur_id = 1;
    self->foreground = NULL;
    return self;
}

static void
job_dir_register_job_internal (job_dir_t *self,
                               job_t     *job)
{
    job_t *iter;

    printf ("\n[%d]   %d\n", job->id, job->pid);

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
job_dir_register_job (job_dir_t    *self,
                      invocation_t *invocation,
                      pid_t         pid)
{
    job_t *job;

    job = calloc (1, sizeof (job_t));
    job->pid = pid;
    job->id = self->cur_id++;
    job->invocation = invocation_copy (invocation);

    job_dir_register_job_internal (self, job);
}

bool
job_dir_run_as_foreground (job_dir_t *self,
                           int        id)
{
    job_t *iter;
    job_t *prev;
    bool found;

    if (self->foreground) {
        job_dir_suspend_foreground (self);
    }

    prev = NULL;
    iter = self->jobs;

    found = FALSE;

    while (iter != NULL) {
        if (iter->id == id) {
            found = TRUE;
        }

        // id of -1 affects most recent job
        if (id == -1 && iter->next == NULL) {
            found = TRUE;
        }

        if (found) {
            self->foreground = iter;
            self->foreground->id = FOREGROUND_JOB_ID;
            if (--self->n_jobs == 0) {
                self->cur_id = 1;
            }

            if (prev) {
                prev->next = iter->next;
            }
            else {
                self->jobs = iter->next;
            }

            kill (self->foreground->pid, SIGCONT);
            waitpid (self->foreground->pid, NULL, WUNTRACED);
            return TRUE;
        }

        prev = iter;
        iter = iter->next;
    }

    return FALSE;
}

bool
job_dir_run_as_background (job_dir_t *self,
                           int        id)
{
    job_t *iter;

    iter = self->jobs;
    while (iter != NULL) {
        if (iter->id == id) {
            kill (iter->pid, SIGCONT);
            return TRUE;
        }

        // id of -1 affects most recent job
        if (id == -1 && iter->next == NULL) {
            kill (iter->pid, SIGCONT);
            return TRUE;
        }

        iter = iter->next;
    }

    return FALSE;
}

bool
job_dir_kill (job_dir_t *self,
              int        id)
{
    job_t *iter;

    iter = self->jobs;

    while (iter != NULL) {
        if (iter->id == id) {
            kill (iter->pid, SIGKILL);
            waitpid (iter->pid, NULL, 0);
            return TRUE;
        }

        // id of -1 affects most recent job
        if (id == -1 && iter->next == NULL) {
            kill (iter->pid, SIGKILL);
            waitpid (iter->pid, NULL, 0);
            return TRUE;
        }

        iter = iter->next;
    }

    return FALSE;
}

void
job_dir_new_in_foreground (job_dir_t    *self,
                           invocation_t *invocation,
                           pid_t         pid)
{
    if (self->foreground) {
        invocation_free (self->foreground->invocation);
        free (self->foreground);
    }

    job_t *job;

    job = calloc (1, sizeof (job_t));
    job->pid = pid;
    job->id = FOREGROUND_JOB_ID;
    job->invocation = invocation_copy (invocation);

    self->foreground = job;

    waitpid (pid, NULL, WUNTRACED);
}

void
job_dir_suspend_foreground (job_dir_t *self)
{
    if (!self->foreground)
        return;

    // Suspend Process
    kill (self->foreground->pid, SIGSTOP);

    // Register as proper job
    self->foreground->id = self->cur_id++;
    job_dir_register_job_internal (self, self->foreground);

    // Unset foreground
    self->foreground = NULL;
}

void
job_dir_flush (job_dir_t *self)
{
    job_t *iter;
    job_t *prev;

    iter = self->jobs;
    prev = NULL;

    while (iter != NULL) {

        // remove dirty iterator elements
        if (iter->dirty) {
            job_t *after;

            after = iter->next;

            if (!prev) {
                self->jobs = after;
            } else {
                prev->next = after;
            }

            invocation_free (iter->invocation);
            free (iter);
            if (--self->n_jobs == 0) {
                self->cur_id = 1;
            }

            iter = after;
            continue;
        }

        prev = iter;
        iter = iter->next;
    }
}

void
job_dir_kill_all (job_dir_t *self)
{
    // kill all jobs and die
    if (self->foreground) {
        kill (self->foreground->pid, SIGKILL);
    }

    job_t *iter;

    iter = self->jobs;
    while (iter != NULL) {
        kill (iter->pid, SIGKILL);
        waitpid (iter->pid, NULL, 0);
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
        process_get_state (iter->pid);
        if (waitpid (iter->pid, NULL, WNOHANG)) {
            char *string;
            // job state has changed
            // flag job for removal
            iter->dirty = TRUE;

            string = tokens_to_string (iter->invocation->tokens);
            printf ("[%d] <Done>  %s\n", iter->id, string);
            free (string);
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

        const char *state;
        char *command;

        switch (process_get_state (iter->pid))
        {
        case PROCESS_STATE_IDLE:
            state = "Idle";
            break;
        case PROCESS_STATE_ZOMBIE:
            state = "Zombie";
            break;
        case PROCESS_STATE_RUNNABLE:
            state = "Runnable";
            break;
        case PROCESS_STATE_SLEEPING:
            state = "Sleeping";
            break;
        case PROCESS_STATE_STOPPED:
            state = "Stopped";
            break;
        default:
            state = "Unknown";
        }

        command = tokens_to_string (iter->invocation->tokens);
        printf ("[%d] <%s>  %s\n", iter->id, state, command);
        free (command);
    }

    printf("\n");
}
