#include "common.h"

#include "input.h"
#include "history.h"
#include "builtin.h"

#include <libgen.h>
#include <sys/wait.h>

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
execute (char **tokens)
{
    int result;

    result = execvp (tokens[0], tokens);
    if (result == -1)
    {
        printf ("No process '%s' \n", tokens[0]);
    }

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
dispatch (state_t  *state,
          char    **tokens)
{
    pid_t pid;

    if (!tokens || !(*tokens)) {
        return;
    }

    history_push (state->history, tokens);

    if (handle_builtin (state, tokens)) {
        return;
    }

    pid = fork();

    if (pid == 0) {
        int result;

        result = execute (tokens);
        exit (result);
    }

    waitpid (pid, NULL, 0);
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
        int n_tokens;
        char **tokens;
        char *input;

        print_prompt ();
        input = get_input ();
        tokens = parse_input (input, &n_tokens);

        dispatch (&state, tokens);
    }
}
