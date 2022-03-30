// history.c
// Matthew Jakeman (mjak923)

#include "history.h"

#define MAX_HISTORY 10

typedef struct history_entry_t
{
    int id;
    invocation_t *invocation;
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
history_entry_new (int           id,
                   invocation_t *invocation)
{
    history_entry_t *entry;

    entry = calloc (1, sizeof (history_entry_t));
    entry->id = id;
    entry->invocation = invocation_copy (invocation);
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

    invocation_free (entry->invocation);
    free (entry);

    *pointer = NULL;
}

struct history_t
{
    history_entry_t *queue;
    int n_entries;
    int cur_index;
};

/**
 * history_new:
 *
 * Creates a new #history_t structure.
 *
 * Returns: New #history_t struct
 */
history_t *
history_new ()
{
    history_t *history;

    history = calloc (1, sizeof (history_t));
    history->cur_index = 1;
    return history;
}

/**
 * history_get_max:
 *
 * Gets the maximum number of history entries
 *
 * @self: History object
 * @min: (out) Integer with minimum index
 * @max: (out) Integer with maximum index
 */
void
history_get_range (history_t *self,
                   int       *min,
                   int       *max)
{
    if (self->n_entries == 0) {
        *min = 0;
        *max = 0;
        return;
    }

    *min = self->cur_index - self->n_entries;
    *max = self->cur_index - 1;
}

/**
 * history_get_invocation:
 *
 * Retrieve invocation for given index from history.
 *
 * @self: History object
 * @id: History index to lookup
 *
 * Returns: Invocation (owned by #history_t instance)
 *
 */
invocation_t *
history_get_invocation (history_t *self,
                        int        id)
{
    int min, max;
    history_entry_t *iter;

    history_get_range (self, &min, &max);

    if (id >= min && id <= max) {
        for (iter = self->queue;
             iter != NULL;
             iter = iter->next) {
            if (iter->id == id) {
                return iter->invocation;
            }
        }
    }

    return NULL;
}

/**
 * history_print:
 *
 * Prints out the last ten items in history.
 *
 */
void
history_print (history_t *self)
{
    history_entry_t *iter;

    if (!self->queue) {
        printf ("No items in history\n");
        return;
    }

    for (iter = self->queue;
         iter != NULL;
         iter = iter->next)
    {
        char *command;

        command = tokens_to_string (iter->invocation->tokens);
        printf ("%d : %s\n", iter->id, command);
        free (command);
    }
}

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
 * @invocation a command invocation
 *
 */
void
history_push (history_t    *self,
              invocation_t *invocation)
{
    history_entry_t *entry;

    entry = history_entry_new (self->cur_index++, invocation);

    if (self->n_entries >= MAX_HISTORY)
        history_dequeue (self);

    history_queue (self, entry);
}

void
history_transform (history_t     *self,
                   invocation_t **command)
{
    int index;
    int min;
    int max;
    char **tokens;

    tokens = (*command)->tokens;

    if (strcmp (tokens[0], "history") != 0) {
        return;
    }

    if (!tokens[1]) {
        return;
    }

    index = atoi (tokens[1]);

    history_get_range (self, &min, &max);
    if (index >= min && index <= max) {
        invocation_t *invocation;
        invocation = history_get_invocation (self, index);

        invocation_free (*command);
        *command = invocation_copy (invocation);
        return;
    }

    printf ("Argument must be a number between %d and %d\n", min, max);

    return;
}
