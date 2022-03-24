#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define bool int
#define FALSE 0
#define TRUE 1

#define TOKEN_ARRAY_SIZE 100

void print_prompt ()
{
    printf ("# ");
}

char *
get_input ()
{
    char *buffer = NULL;
    size_t buffer_size = 0;
    int length;

    // Read in input up to a newline '\n' character
    length = getline (&buffer, &buffer_size, stdin);

    if (length == -1) {
        fprintf (stderr, "Unable to read line\n");
        return NULL;
    }

    // getline includes the newline character but no null
    // termination. Replace the '\n' with '\0'.
    if ((length > 0) && (buffer[length - 1] == '\n')) {
        buffer[length - 1] = '\0';
    }

    return buffer;
}

char **
parse_input (char  *input,
             char **command,
             int   *n_tokens)
{
    char *buffer;
    char **tokens;
    char *cur_token;
    int index;

    // Input is a string ending with \n
    if (!input)
        goto error;

    // Get command token
    cur_token = strtok (input, " \t");
    *command = cur_token;

    if (*command == NULL)
        goto error;

    // Get argument tokens
    tokens = malloc (sizeof (char *) * TOKEN_ARRAY_SIZE);
    index = 0;

    while (TRUE)
    {
        cur_token = strtok (NULL, " \t");

        if (!cur_token)
            break;

        tokens[index++] = cur_token;
    }

    *n_tokens = index+1;
    return tokens;

error:
    *n_tokens = 0;
    *command = NULL;
    return NULL;
}

int
execute (char  *command,
         char **tokens)
{
    int result;

    result = execvp (command, tokens);
    if (result == -1)
    {
        printf ("No process '%s' \n", command);
    }

    return result;
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
        char *input;
        pid_t pid;

        print_prompt ();
        input = get_input ();
        tokens = parse_input (input, &command, &n_tokens);

        pid = fork();
        if (pid == 0)
        {
            int result;

            result = execute (command, tokens);
            return result;
        }

        waitpid (pid, NULL, 0);

        free (tokens);
        free (command);
    }
}
