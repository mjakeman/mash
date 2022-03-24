#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define bool int
#define FALSE 0
#define TRUE 1

void print_prompt ()
{
    printf ("# ");
}

char **parse_input (char **command, int *n_tokens)
{
    char *buffer = NULL;
    size_t buffer_size = 0;

    getline (&buffer, &buffer_size, stdin);

    // a string with lots of spaces in it
    //  -> an array of strings with no whitespace
    //  -> string.split(my_str, " \t")

    char *command_token;

    command_token = strtok (buffer, " \t");
    if (command_token == NULL)
    {
        *n_tokens = 0;
        *command = NULL;
        return NULL;
    }

    *command = command_token;

    char **tokens = malloc (sizeof(char*) * 100);
    char *cur_token = command_token;

    int index = 0;
    while (cur_token)
    {
        cur_token = strtok (NULL, " \t");
        tokens[index++] = cur_token;
    }

    *n_tokens = index+1;
    return tokens;
}

int main ()
{
    printf ("mAsh! Matthew's Shell\n");

    bool running = TRUE;

    while (running)
    {
        int n_tokens;
        char **tokens;
        char *command;

        print_prompt ();
        tokens = parse_input (&command, &n_tokens);

        pid_t pid = fork();
        if (pid == 0)
        {
            // we are the child
            int result = execvp (command, tokens);

            if (result == -1)
            {
                printf ("No process '%s' \n", command);
                return 1;
            }

            return 0;
        }

        waitpid (pid, NULL, 0);

        free (tokens);
    }
}
