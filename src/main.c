#include "common.h"

#include "input.h"

#include <libgen.h>
#include <sys/wait.h>

#define BUFFER_SIZE 500

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

bool
handle_builtin (char **tokens)
{
    if (strcmp (tokens[0], "cd") == 0)
    {
        chdir (tokens[1]);
        return TRUE;
    }

    return FALSE;
}

int main ()
{
    printf ("mAsh! Matthew's Shell\n");

    bool running = TRUE;

    while (running)
    {
        int n_tokens;
        char **tokens;
        char *input;
        pid_t pid;

        print_prompt ();
        input = get_input ();
        tokens = parse_input (input, &n_tokens);

        if (handle_builtin (tokens))
        {
            free (tokens);
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

        free (tokens);
    }
}
