#include <stdio.h>

#define bool int
#define FALSE 0
#define TRUE 1

void print_prompt ()
{
    printf ("# ");
}

char **parse_input (int *n_tokens)
{
    char *buffer = NULL;
    size_t buffer_size = 0;

    size_t length = getline (&buffer, &buffer_size, stdin);

    printf ("length: %ld / capacity: %ld \n", length, buffer_size);

    *n_tokens = 0;
    return NULL;
}

int main ()
{
    printf ("mAsh! Matthew's Shell\n");

    bool running = TRUE;

    while (running)
    {
        int n_tokens;

        print_prompt ();
        parse_input (&n_tokens);
    }
}
