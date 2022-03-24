#include "common.h"

#include "input.h"
#include "history.h"

#include <libgen.h>
#include <sys/wait.h>

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

typedef struct
{
    char *home_dir;
    history_t *history;
} state_t;

bool
handle_builtin (state_t  *state,
                char    **tokens)
{
    if (strcmp (tokens[0], "cd") == 0)
    {
        char *arg = tokens[1];

        if (!arg) {
            chdir (state->home_dir);
            return TRUE;
        }

        if (chdir (arg) == -1) {
            printf ("No such directory '%s'\n", arg);
        }

        return TRUE;
    }
    else if (strcmp (tokens[0], "h") == 0)
    {
        // TODO: Handle number argument
        // TODO: Handle long form name 'history'
        history_print (state->history);
        return TRUE;
    }
    else if (strcmp (tokens[0], "exit") == 0)
    {
        exit (EXIT_SUCCESS);
        return TRUE; // will never return
    }

    return FALSE;
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
        pid_t pid;

        print_prompt ();
        input = get_input ();
        tokens = parse_input (input, &n_tokens);

        history_push (state.history, tokens);

        if (handle_builtin (&state, tokens))
        {
            continue;
        }

        pid = fork();
        if (pid == 0)
        {
            int result;

            result = execute (tokens);
            return result;
        }

        waitpid (pid, NULL, 0);
    }
}
