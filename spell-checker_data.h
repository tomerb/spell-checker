#ifndef __SPELL_CHECKER_DATA_H
#define __SPELL_CHECKER_DATA_H

/**
 * A data model for storing dictionary words.
 */

struct _SpellCheckerData;
typedef struct _SpellCheckerData *SpellCheckerDataHandle;

/**
 * Initialize the data model.
 * @return a handle to the data model.
 */
SpellCheckerDataHandle scdInit(void);

/**
 * Finalize the data model, releasing all the resources attached to it.
 */
void scdFinalize(SpellCheckerDataHandle data);

/**
 * Add a word to the dictionary.
 * @param data a handle to the current data model.
 * @param word the word to add.
 * @return 0 on success, -1 on failure.
 */
int scdAddWord(SpellCheckerDataHandle data, const char *word);

/**
 * Check if the dictionary contains the word given.
 * @param data a handle to the current data model.
 * @param word the word to check for existance in the dictionary.
 * @return 0 on success, -1 on failure.
 */
int scdHasWord(SpellCheckerDataHandle data, const char *word);

#endif
