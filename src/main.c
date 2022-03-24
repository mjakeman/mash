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
             int   *n_tokens)
{
    int index;
    char **tokens;
    char *cur_token;

    // Input is a string ending with \n
    if (!input) {
        *n_tokens = 0;
        return NULL;
    }

    // Allocate tokens
    tokens = malloc (sizeof (char *) * TOKEN_ARRAY_SIZE);
    index = 0;

    // Get command token
    cur_token = strtok (input, " \t");

    // Get argument tokens
    while (cur_token) {
        tokens[index++] = cur_token;
        cur_token = strtok (NULL, " \t");

        if (index > TOKEN_ARRAY_SIZE) {
            fprintf (stderr, "Exceeded maximum number of arguments (%d)\n", TOKEN_ARRAY_SIZE);
            break;
        }
    }

    *n_tokens = index+1;
    return tokens;
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
