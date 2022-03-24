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

    printf ("[%s]# ", cur_dir);
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
    history_t *history;
} state_t;

bool
handle_builtin (state_t  *state,
                char    **tokens)
{
    if (strcmp (tokens[0], "cd") == 0)
    {
        chdir (tokens[1]);
        return TRUE;
    }
    else if (strcmp (tokens[0], "h") == 0)
    {
        // TODO: Handle number argument
        history_print (state->history);
        return TRUE;
    }

    return FALSE;
}

int main ()
{
    printf ("mAsh! Matthew's Shell\n");

    bool running;
    state_t state;
    state.history = history_new ();

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

        if (handle_builtin (&state, tokens))
        {
            history_push (state.history, tokens);
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
        history_push (state.history, tokens);
    }
}
