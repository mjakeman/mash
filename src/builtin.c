// builtin.c
// Matthew Jakeman (mjak923)

#include "builtin.h"

/**
 * builtin_run_chdir:
 *
 * Builtin command for changing the current working directory.
 *
 * @tokens: String array of tokens
 * @home_dir: Absolute path to home directory
 *
 * Returns: Whether the builtin was handled successfully
 *
 */
bool
builtin_run_chdir (char **tokens,
                   char  *home_dir)
{
    char *arg;

    arg = tokens[1];

    if (!arg) {
        chdir (home_dir);
        return TRUE;
    }

    if (chdir (arg) == -1) {
        printf ("No such directory '%s'\n", arg);
    }

    return TRUE;
}

/**
 * builtin_run_history:
 *
 * Builtin command for viewing and replaying history.
 *
 * @tokens: String array of tokens
 * @state: Opaque shell state structure
 * @history: History object
 *
 * Returns: Whether the builtin was handled successfully
 *
 */
bool
builtin_run_history (char      **tokens,
                     state_t    *state,
                     history_t  *history)
{
    char *arg;
    int index;
    int min;
    int max;

    arg = tokens[1];

    if (!arg) {
        history_print (history);
        return TRUE;
    }

    history_get_range (history, &min, &max);

    index = atoi (arg);
    if (index >= min && index <= max) {
        char **tokens;
        tokens = history_get_tokens (history, index);

        // todo: pass in invocation
        dispatch (state, tokens);
        return TRUE;
    }

    printf ("Argument must be a number between %d and %d\n", min, max);

    return TRUE;
}
