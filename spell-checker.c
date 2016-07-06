#include <stdlib.h>

#include "spell-checker.h"
#include "spell-checker_runner.h"

typedef struct _SpellCheckerDictionary
{
    SpellCheckerRunnerHandle runner;
} _SpellCheckerDictionary;

SpellCheckerDictionaryHandle createSpellCheckerDictionary()
{
    SpellCheckerDictionaryHandle dict = malloc(sizeof(_SpellCheckerDictionary));
    if (dict)
    {
        dict->runner = scrInit();
        if (!dict->runner)
        {
            free(dict);
            return NULL;
        }
    }
    return dict;
}

int closeSpellCheckerDictionary(SpellCheckerDictionaryHandle dict)
{
    if (dict)
    {
        scrFinalize(dict->runner);
        free(dict);
    }
    return 0;
}

int spellCheckerAddWord(SpellCheckerDictionaryHandle dict, const char *word)
{
    if (!dict || !word)
    {
        return -1;
    }

    return scrAddWord(dict->runner, word);
}

void spellCheck(SpellCheckerDictionaryHandle dict, const char *text,
                SpellCheckerCallback callback)
{
    scrRunSpellCheck(dict->runner, text, callback);
}
