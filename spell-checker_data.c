#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "spell-checker_data.h"

/**
 * A Trie implementation to hold the dictionary data.
 *
 * See https://en.wikipedia.org/wiki/Trie for a description of a Trie.
 *
 * TODO: Extract the implementation out, put into specific implementation file,
 * to easily test with other data-structures.
 *
 * This implementation is a bit wasteful in memory, but should be rather fast.
 * It is wasteful, as it is allocating an empty array for each node created,
 * even though potentially none of the children will be used. In practice, most
 * of them will probably not be used (TODO: measure how much exactly).
 * It is, however, rather fast, as the access to each element is O(1), as each
 * child is located at the index of it's character representation.
 *
 * Visual representation of the Trie containing a single word 'air':
 *
 *                   (0)            <-- Root
 *              ______|______
 *              |     |      |
 *             (0)...(a)...(0xFF)   <-- ... 256 children (all but one empty)
 *              ______|_______
 *              |     |      |
 *             (0)...(i)...(0xFF)   <-- ... 256 children (all but one empty)
 *              ______|_______
 *              |     |      |
 *             (0)...(r)...(0xFF)   <-- ... 256 children (all but one empty)
 *              ______|_______
 *              |            |
 *             (0)         (0xFF)   <-- ... 256 children (all empty)
 *
 *
 * Multiple strategies may be implemented to improve memory waste: allocating
 * nodes for legal characters only, allocating nodes as-needed only, not
 * allocating nodes for the leafs, etc.. All have different pros and cons.
 *
 * Each node in the Trie holds a single character and a list of children.
 * The root node has its character element set to 0.
 * Upon initialization, only the root node is created, with its children's
 * array zero-ed out.
 * Every time a word is added, a new node is created in place of the appropriate
 * child node.
 */

/*
 *Max ascii values (with extended characters)
 */
#define MAX_CHARS_PER_NODE 256

/*
 * Each data node holds a character and an array of MAX_CHARS_PER_NODE children.
 */
struct _SpellCheckerData
{
    char chr;
    struct _SpellCheckerData *child;
};

/*
 * Recursively delete a node.
 * Note: the node itself is not deleted, but all its children does.
 */
static void scdDeleteNode(SpellCheckerDataHandle node)
{
    if (node)
    {
        if (node->child)
        {
            for(int i = 0; i < MAX_CHARS_PER_NODE; ++i)
                scdDeleteNode(&node->child[i]);
            free(node->child);
            node->child = NULL;
        }
    }
}

/*
 * Initialize a node with the given character, and allocate memory for the
 * children array.
 */
static int scdInitNode(char chr, SpellCheckerDataHandle node)
{
    node->chr = chr;

    node->child = calloc(MAX_CHARS_PER_NODE, sizeof(struct _SpellCheckerData));
    if (!node->child)
    {
        printf("Failed allocating memory for child array\n");
        return -1;
    }

    return 0;
}

static SpellCheckerDataHandle scdCreateNode(void)
{
    SpellCheckerDataHandle node = malloc(sizeof(struct _SpellCheckerData));
    if (!node)
    {
        printf("Failed allocating memory for node\n");
        return NULL;
    }

    if (-1 == scdInitNode(0, node))
    {
        free(node);
        return NULL;
    }

    return node;
}

SpellCheckerDataHandle scdInit(void)
{
    return scdCreateNode();
}

void scdFinalize(SpellCheckerDataHandle data)
{
    scdDeleteNode(data);
    free(data);
}

/*
 * Return an unsigned char instead of a signed one, and transform upper-case
 * letters to lower-case.
 */
static inline unsigned char scdNormalizeChar(char c)
{
    if (('A' <= c) && ('Z' >= c))
        return c + 0x20;
    return c;
}

/*
 * Make sure a word is valid, per the requirements given in spell-checker.h.
 */
static inline int scdIsValid(const char *word)
{
    for (size_t i = 0; i < strlen(word); ++i)
    {
        if (!( (('a' <= word[i]) && ('z' >= word[i])) ||
               (('A' <= word[i]) && ('Z' >= word[i])) ||
               (('0' <= word[i]) && ('9' >= word[i])) ||
               (0x80 <= scdNormalizeChar(word[i])) ))
            return 0;
    }
    return 1;
}

int scdAddWord(SpellCheckerDataHandle data, const char *word)
{
    if (!data || !word || !data->child)
    {
        printf("Invalid arguments when trying to add a word\n");
        return -1;
    }

    if (!scdIsValid(word))
    {
#ifdef DEBUG
        printf("Attempted to add an invalid word (%s) to dictionary\n", word);
#endif
        return -1;
    }

    SpellCheckerDataHandle node = data;
    size_t i = 0;
    const size_t n = strlen(word);

    /* Find existing nodes (characters) */
    while ((i < n) && (0 != node->child[scdNormalizeChar(word[i])].chr))
    {
        node = &node->child[scdNormalizeChar(word[i])];
        ++i;
    }

    /* If word is longer than what we have stored, we need to add each letter */
    while (i < n)
    {
        if (-1 == scdInitNode(scdNormalizeChar(word[i]),
                              &node->child[scdNormalizeChar(word[i])]))
        {
            /* TODO: cleanup nodes that were already created */
            return -1;
        }
        node = &node->child[scdNormalizeChar(word[i])];
        ++i;
    }

    return 0;
}

int scdHasWord(SpellCheckerDataHandle data, const char *word)
{
    if (!data || !word || !data->child)
    {
        printf("Invalid arguments passed to scdHasWord\n");
        return -1;
    }

    SpellCheckerDataHandle node = data;
    const size_t n = strlen(word);
    size_t i = 0;
    while (i < n)
    {
        if (0 == node->child[scdNormalizeChar(word[i])].chr)
            return 0;
        else
            node = &node->child[scdNormalizeChar(word[i])];
        ++i;
    }

    return (i == n);
}
