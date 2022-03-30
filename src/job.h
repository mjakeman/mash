// job.h
// Matthew Jakeman (mjak923)

#include "common.h"

#include "invocation.h"

struct job_t;
typedef struct job_t job_t;

struct job_dir_t;
typedef struct job_dir_t job_dir_t;

job_dir_t *
job_dir_new ();

void
job_dir_register_job (job_dir_t    *self,
                      invocation_t *invocation,
                      pid_t         pid);

void
job_dir_flush (job_dir_t *self);

void
job_dir_iterate (job_dir_t *self);

void
job_dir_print_all (job_dir_t *self);

void
job_dir_suspend_foreground (job_dir_t *self);

void
job_dir_new_in_foreground (job_dir_t    *self,
                           invocation_t *invocation,
                           pid_t         pid);

bool
job_dir_run_as_foreground (job_dir_t *self,
                           int        id);

bool
job_dir_run_as_background (job_dir_t *self,
                           int        id);

bool
job_dir_kill (job_dir_t *self,
              int        id);

void
job_dir_kill_all (job_dir_t *self);
