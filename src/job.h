// job.h
// Matthew Jakeman (mjak923)

#include "common.h"

struct job_t;
typedef struct job_t job_t;

struct job_dir_t;
typedef struct job_dir_t job_dir_t;

job_dir_t *
job_dir_new ();

void
job_dir_register_job (job_dir_t *self,
                      pid_t      pid);

void
job_dir_flush (job_dir_t *self);

void
job_dir_iterate (job_dir_t *self);

void
job_dir_print_all (job_dir_t *self);
