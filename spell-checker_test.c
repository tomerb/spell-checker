#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "spell-checker.h"

#define DICTIONARY_FILE "dictionary.txt"
#define TEST_FILE "trie.txt"

static long getFileSize(FILE *file)
{
    if (-1 == fseek(file, 0, SEEK_END))
    {
        printf("Failed seeking to end of file\n");
        return -1;
    }
    const long fsize = ftell(file);
    if (-1 == fsize)
    {
        printf("Failed finding file size\n");
        return -1;
    }
    rewind(file);
    return fsize;
}

static SpellCheckerDictionaryHandle loadDictionary(const char *dictFileName)
{
    FILE *dictFile = fopen(dictFileName, "r");
    if (!dictFile)
    {
        printf("Failed opening dictionary file (%s)\n", dictFileName);
        return NULL;
    }

    const long fsize = getFileSize(dictFile);
    if (-1 == fsize)
    {
        printf("Failed finding dictionary file size\n");
        fclose(dictFile);
        return NULL;
    }

    SpellCheckerDictionaryHandle dict = createSpellCheckerDictionary();
    if (!dict)
    {
        printf("Failed creating dictionary\n");
        fclose(dictFile);
        return NULL;
    }

    printf("Loading words to dictionary...\n");

    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    ssize_t totalRead = 0;
    size_t numWords = 0;
    while ((read = getline(&line, &len, dictFile)) != -1)
    {
        if (SIZE_MAX == numWords)
        {
            printf("\nReached maximum size. Will not add more words\n");
            break;
        }
        line[read-1] = '\0';
        if (-1 == spellCheckerAddWord(dict, line))
        {
            printf("\nFailed adding word to spell checker. Bailing...\n");
            closeSpellCheckerDictionary(dict);
            fclose(dictFile);
            return NULL;
        }
        totalRead += read;
        ++numWords;

        if (0 == (totalRead % 10000))
        {
            printf("%d%%\r", totalRead*100/fsize);
            fflush(stdout);
        }
    }

    free(line);
    fclose(dictFile);

    printf("100%\n%zu words added to dictionary\n", numWords);

    return dict;
}

void callback(const char *word)
{
    printf("Got a misspelled word: %s\n", word);
}

static int testSpellChecker(SpellCheckerDictionaryHandle dict)
{
    FILE *file = fopen(TEST_FILE, "r");
    if (!file)
    {
        printf("Failed opening file '%s'\n", TEST_FILE);
        return -1;
    }

    const long fsize = getFileSize(file);
    if (-1 == fsize)
    {
        printf("Failed finding test file size\n");
        fclose(file);
        return -1;
    }

    char *strToTest = malloc(fsize+1);
    if (!strToTest)
    {
        printf("Failed malloc-ing memory\n");
        fclose(file);
        return -1;
    }
    const size_t bread = fread(strToTest, 1, fsize, file);
    fclose(file);
    if (bread != fsize)
    {
        printf("Failed reading from file (%zu/%zu bytes read)\n", bread, fsize);
        free(strToTest);
        return -1;
    }
    strToTest[fsize] = '\0';
    spellCheck(dict, strToTest, callback);
    sleep(5);
    free(strToTest);
    return 0;
}

int main(void)
{
    /* TODO: load words to same dictionary from multiple thread to test and
       validate multi-threaded code. */
    SpellCheckerDictionaryHandle dict = loadDictionary(DICTIONARY_FILE);
    if (!dict)
    {
        printf("Failed loading dictionary into spell-checker!\n");
        return -1;
    }

    if (0 == testSpellChecker(dict))
    {
        printf("+++ All tests passed! +++\n");
    }
    else
    {
        printf("--- Test(s) failed! ---\n");
    }

    closeSpellCheckerDictionary(dict);

    return 0;
}
