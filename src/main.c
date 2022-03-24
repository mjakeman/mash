#include "common.h"

#include "input.h"

#include <libgen.h>
#include <sys/wait.h>

#define BUFFER_SIZE 500
#define MAX_HISTORY 10

void print_prompt ()
{
    char *cur_dir;
    char cwd[BUFFER_SIZE];

    cur_dir = "error";
    if (getcwd (cwd, BUFFER_SIZE))
        cur_dir = basename (cwd);

    printf ("[%s]# ", cur_dir);
}

char *
tokens_to_string (char **tokens)
{
    int buffer_size;
    char *output;
    char *token;
    int index;
    int cur_length;

    if (!tokens || !tokens[0])
        return NULL;

    buffer_size = BUFFER_SIZE;
    output = malloc (sizeof (char) * buffer_size);

    index = 0;
    token = tokens[index];
    cur_length = 0;
    while (token) {
        int delta_length;

        delta_length = strlen (token);

        // check we haven't exceeded the maximum capacity of the buffer
        if (cur_length + delta_length > buffer_size) {
            buffer_size *= 2;
            output = realloc (output, buffer_size);
        }

        // append string
        strcat (output, token);

        cur_length += delta_length;
    }

    return output;
}

typedef struct history_entry_t
{
    int id;
    char **tokens;
    struct history_entry_t *next;
} history_entry_t;

/**
 * history_entry_new:
 *
 * Allocates a new #history_entry_t structure
 *
 * @id: Unique id for this entry
 * @tokens: String array of tokens (this function takes ownership)
 *
 */
history_entry_t *
history_entry_new (int    id,
                   char **tokens)
{
    history_entry_t *entry;

    entry = malloc (sizeof (history_entry_t));
    entry->id = id;
    entry->tokens = tokens;
    entry->next = NULL;

    return entry;
}

/**
 * history_entry_clear:
 *
 * Frees the memory of a history entry and clears the pointer.
 *
 * @pointer: Double pointer to a #history_entry_t.
 *
 */
void
history_entry_clear (history_entry_t **pointer)
{
    history_entry_t *entry;

    entry = *pointer;

    free (entry->tokens);
    free (entry);

    *pointer = NULL;
}

typedef struct
{
    history_entry_t *queue;
    int n_entries;
    int cur_index;
} history_t;

/**
 * history_queue:
 *
 * Appends a #history_entry_t to the end of the queue
 *
 * @self pointer to history object
 * @entry entry to be appended
 *
 */
static void
history_queue (history_t       *self,
               history_entry_t *entry)
{
    history_entry_t *iter;

    // queue is empty
    if (!self->queue) {
        self->queue = entry;
        self->n_entries++;
        return;
    }

    // iterate until the end of the queue
    for (iter = self->queue;
         iter->next != NULL;
         iter = iter->next) {}

    // append
    iter->next = entry;
    self->n_entries++;
}

/**
 * history_dequeue:
 *
 * Removes the first #history_entry_t in the queue
 *
 * @self pointer to history object
 *
 */
static void
history_dequeue (history_t *self)
{
    history_entry_t *old;

    if (!self->queue) {
        fprintf (stderr, "Could not dequeue history as queue is empty\n");
        return;
    }

    old = self->queue;
    self->queue = self->queue->next;
    self->n_entries--;

    history_entry_clear (&old);
}

/**
 * history_push:
 *
 * Pushes a command to the history queue, removing the oldest element if
 * total entries exceeds #MAX_HISTORY.
 *
 * @self pointer to history object
 * @tokens string array of tokens
 *
 */
void
history_push (history_t  *self,
              char      **tokens)
{
    history_entry_t *entry;

    entry = history_entry_new (self->cur_index++, tokens);

    if (self->n_entries >= MAX_HISTORY)
        history_dequeue (self);

    history_queue (self, entry);
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

    bool running;
    history_t history;

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

        if (handle_builtin (tokens))
        {
            history_push (&history, tokens);
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
        history_push (&history, tokens);
    }
}
