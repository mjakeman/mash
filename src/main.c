#include "common.h"

#include "input.h"
#include "history.h"
#include "builtin.h"
#include "invocation.h"

#include <libgen.h>
#include <sys/wait.h>

#define PIPE_READ 0
#define PIPE_WRITE 1

struct state_t
{
    char *home_dir;
    history_t *history;
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

int
execute (char **tokens,
         int    n_tokens)
{
    int result;
    char **tokens_cpy;

    // copy as null terminated array
    tokens_cpy = malloc (sizeof (char *) * (n_tokens + 1));
    memcpy (tokens_cpy, tokens, n_tokens * sizeof (char *));
    tokens_cpy [n_tokens] = NULL;

    // execute command and tokens
    result = execvp (tokens_cpy[0], tokens_cpy);
    if (result == -1)
    {
        printf ("No process '%s' \n", tokens_cpy[0]);
    }

    free (tokens_cpy);

    return result;
}

bool
handle_builtin (state_t  *state,
                char    **tokens)
{
    if (strcmp (tokens[0], "exit") == 0) {
        exit (EXIT_SUCCESS);
    }

    if (strcmp (tokens[0], "cd") == 0) {
        return builtin_run_chdir (tokens, state->home_dir);
    }

    if ((strcmp (tokens[0], "h") == 0) ||
        (strcmp (tokens[0], "history") == 0)) {
        // TODO: Handle equivalent history index (i.e. on the fourth command, issuing 'h 4')
        return builtin_run_history (tokens, state, state->history);
    }

    return FALSE;
}

void
dispatch (state_t      *state,
          invocation_t *invocation)
{
    command_t *command;
    pid_t pid;

    int old_pipe_fds[2];
    int new_pipe_fds[2];

    bool has_prev_command;
    bool has_next_command;

    has_prev_command = FALSE;

    // history_push (state->history, tokens);

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
            execute (&invocation->tokens[command->index], command->n_tokens);
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

    waitpid (pid, NULL, 0);

    /*if (handle_builtin (state, tokens)) {
        return;
    }*/
}

int main ()
{
    printf ("mAsh! Matthew's Shell\n");

    bool running;
    state_t state;
    state.history = history_new ();
    state.home_dir = malloc (sizeof (char) * BUFFER_SIZE);
    getcwd (state.home_dir, BUFFER_SIZE);

    running = TRUE;

    while (running)
    {
        invocation_t *invocation;
        char *input;

        print_prompt ();
        input = get_input ();
        invocation = parse_input (input);

        dispatch (&state, invocation);

        // invocation_free (invocation);
    }
}
