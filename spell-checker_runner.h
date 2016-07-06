#ifndef __SPELL_CHECKER_RUNNER_H
#define __SPELL_CHECKER_RUNNER_H

#include "spell-checker.h"

/**
 * An asynchronous runner for posting spell-checking operations.
 *
 * TODO: might want to unify with spell-checker.c.
 */

struct _SpellCheckerRunner;
typedef struct _SpellCheckerRunner *SpellCheckerRunnerHandle;

/**
 * Initialize the runner.
 * This creates a thread that is waiting for commands.
 */
SpellCheckerRunnerHandle scrInit(void);

/**
 * Finalize the runner.
 * @param runner destroy the given runner. This object should not be used again
 * after calling this function.
 * @return 0 on success, -1 on failure
 */
int scrFinalize(SpellCheckerRunnerHandle runner);

/**
 * Add a word to the dictionary.
 * @param runner the runner to use.
 * @param word the word to add.
 * @return 0 on success, -1 on failure
 */
int scrAddWord(SpellCheckerRunnerHandle runner, const char *word);

/**
 * Run a spell-check on the given text.
 * @param runner the runner to use.
 * @param text the text to check for errors.
 * @param callback the callback to use for notifying about invalid words.
 * @return 0 on success, -1 on failure
 */
int scrRunSpellCheck(SpellCheckerRunnerHandle runner, const char *text,
                     SpellCheckerCallback callback);

#endif
