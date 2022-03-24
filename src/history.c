// history.c
// Matthew Jakeman (mjak923)

#include "history.h"

#define MAX_HISTORY 10

typedef struct history_entry_t
{
    int id;
    char **tokens;
    struct history_entry_t *next;
} history_entry_t;

// Helper method for printing out tokens
// as a single string robustly.
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
    output[0] = '\0';

    index = 0;
    token = tokens[index];
    cur_length = 0;
    while (token) {
        int result;
        int add_length;

        add_length = strlen (token);
        result = sprintf (output, "%s %s", output, token);

        if (result < 0) {
            int delta;

            // Buffer is too small
            // Increase buffer by the larger of BUFFER_SIZE or add_length
            delta = BUFFER_SIZE > add_length
                ? BUFFER_SIZE
                : add_length;

            // Reallocate output buffer
            buffer_size += delta;
            output = realloc (output, buffer_size);

            // Try again
            result = sprintf (output, "%s %s", output, token);

            if (result < 0) {
                fprintf (stderr, "Error appending token - output may be corrupt\n");
                return output;
            }
        }

        cur_length += add_length;
        token = tokens[++index];
    }

    return output;
}

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

    history = malloc (sizeof (history_t));
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
 * history_get_tokens:
 *
 * Retrieve tokens for given index from history.
 *
 * @self: History object
 * @id: History index to lookup
 *
 * Returns: String array of tokens (owned by #history_t instance)
 *
 */
char **
history_get_tokens (history_t *self,
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
                return iter->tokens;
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

        command = tokens_to_string (iter->tokens);
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
