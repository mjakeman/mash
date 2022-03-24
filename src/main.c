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

typedef struct history_entry
{
    int id;
    char *command;
    struct history_entry *next;
} history_entry;

/**
 * history_entry_new:
 *
 * Allocates a new #history_entry structure
 *
 * @id: Unique id for this entry
 * @command: String of the command (this function takes ownership)
 *
 */
history_entry *
history_entry_new (int id, char *command)
{
    history_entry *entry;

    entry = malloc (sizeof (history_entry));
    entry->id = id;
    entry->command = command;
    entry->next = NULL;

    return entry;
}

/**
 * history_entry_clear:
 *
 * Frees the memory of a history entry and clears the pointer.
 *
 * @pointer: Double pointer to a #history_entry.
 *
 */
void
history_entry_clear (history_entry **pointer)
{
    history_entry *entry;

    entry = *pointer;

    free (entry->command);
    free (entry);

    *pointer = NULL;
}

typedef struct
{
    history_entry *queue;
    int n_entries;
    int cur_index;
} history;

/**
 * history_queue:
 * Appends a #history_entry to the end of the queue
 *
 * @self pointer to history object
 * @entry entry to be appended
 *
 */
static void
history_queue (history       *self,
               history_entry *entry)
{
    history_entry *iter;

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
 * Removes the first #history_entry in the queue
 *
 * @self pointer to history object
 *
 */
static void
history_dequeue (history *self)
{
    history_entry *old;

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
 * Pushes a command to the history queue, removing the oldest element if
 * total entries exceeds #MAX_HISTORY.
 *
 * @self pointer to history object
 * @command string of command to be entered
 *
 */
void
history_push (history *self,
              char    *command)
{
    history_entry *entry;

    entry = history_entry_new (self->cur_index++, strdup (command));

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
