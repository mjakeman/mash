// job.c
// Matthew Jakeman (mjak923)

#include "job.h"

#include <sys/wait.h>

struct job_dir_t
{
    int n_jobs;
    int cur_id;
    struct job_t *jobs;
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
    self = malloc (sizeof (job_dir_t));
    return self;
}

void
job_dir_register_job (job_dir_t    *self,
                      invocation_t *invocation,
                      pid_t         pid)
{
    job_t *iter;
    job_t *job;

    job = malloc (sizeof (job_t));
    job->pid = pid;
    job->id = self->cur_id++;
    job->invocation = invocation_copy (invocation);

    printf ("[%d]   %d\n", job->id, job->pid);

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
job_dir_iterate (job_dir_t *self)
{
    job_t *iter;

    // iterates over all jobs to see if any have finished
    for (iter = self->jobs;
         iter != NULL;
         iter = iter->next) {
        if (waitpid (iter->pid, NULL, WNOHANG)) {
            char *string;
            // job state has changed
            // todo: check if this is termination (or stopped/started/etc)

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
        printf ("job id %d / pid %d\n", iter->id, iter->pid);
    }
}
