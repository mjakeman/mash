// process.c
// Matthew Jakeman (mjak923)

#include "process.h"

static int
execute (invocation_t *invocation,
         command_t    *command)
{
    int result;
    char **tokens;

    // get tokens as null terminated array
    tokens = invocation_command_get_tokens (invocation, command);

    if (!tokens)
        return EXIT_FAILURE;

    // execute command and tokens
    result = execvp (tokens[0], tokens);
    if (result == -1)
    {
        printf ("No process '%s' \n", tokens[0]);
    }

    free (tokens);

    return result;
}

enum process_state
process_get_state (pid_t pid)
{
    enum process_state result;
    char path[BUFFER_SIZE];
    FILE *fd;

    char pid_str[BUFFER_SIZE];
    char cmd[BUFFER_SIZE];
    char status[BUFFER_SIZE];

    sprintf (path, "/proc/%d/stat", pid);

    result = PROCESS_STATE_UNKNOWN;

    fd = fopen (path, "r");

    if (fd) {
        fscanf (fd, "%s %s %s", pid_str, cmd, status);

        if (strcmp (status, "S") == 0)
            result = PROCESS_STATE_SLEEPING;

        else if (strcmp (status, "R") == 0)
            result = PROCESS_STATE_RUNNABLE;

        else if (strcmp (status, "T") == 0)
            result = PROCESS_STATE_STOPPED;

        else if (strcmp (status, "D") == 0)
            result = PROCESS_STATE_IDLE;

        else if (strcmp (status, "Z") == 0)
            result = PROCESS_STATE_ZOMBIE;

        fclose (fd);
    }

    return result;
}

pid_t
process_run (invocation_t *invocation)
{
    command_t *command;
    pid_t pid;

    int old_pipe_fds[2];
    int new_pipe_fds[2];

    bool has_prev_command;
    bool has_next_command;

    has_prev_command = FALSE;

    pid = -1;

    // loop over all commands
    for (command = invocation->commands;
         command != NULL;
         command = command->next) {

        has_next_command = (command->next != NULL);

        if (pipe (new_pipe_fds) == -1) {
            fprintf (stderr, "Error creating pipe\n");
            exit (EXIT_FAILURE);
        }

        pid = fork ();

        if (pid == 0) {
            // child process
            if (has_prev_command) {
                // connect to the previous command's stdout
                dup2 (old_pipe_fds[PIPE_READ], STDIN_FILENO);
            }

            if (has_next_command) {
                // setup for the next command's stdin
                dup2 (new_pipe_fds[PIPE_WRITE], STDOUT_FILENO);
            }

            // close all file descriptors
            close (old_pipe_fds[PIPE_READ]);
            close (old_pipe_fds[PIPE_WRITE]);
            close (new_pipe_fds[PIPE_WRITE]);
            close (new_pipe_fds[PIPE_READ]);

            // execute for tokens
            execute (invocation, command);
            exit (EXIT_FAILURE);
        }

        if (has_prev_command) {
            // close all pipes on the parent
            close (old_pipe_fds[PIPE_READ]);
            close (old_pipe_fds[PIPE_WRITE]);
        }

        // set old pipe to current pipe and repeat
        old_pipe_fds[PIPE_READ] = new_pipe_fds[PIPE_READ];
        old_pipe_fds[PIPE_WRITE] = new_pipe_fds[PIPE_WRITE];

        has_prev_command = TRUE;
    }

    // if we have more than one command then these
    // pipes also need to be closed
    if (has_prev_command) {
        close (new_pipe_fds[PIPE_READ]);
        close (new_pipe_fds[PIPE_WRITE]);
    }

    return pid;
}
