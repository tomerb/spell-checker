#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "spell-checker_runner.h"
#include "spell-checker_data.h"

/**
 * This runner implements a simple message-queue for a simpler handling of async
 * tasks.
 *
 * The runner's main thread will wait until a new message is put on its queue
 * (of tasks). Once a new message is on the queue, a condition is used to signal
 * about the new task, making the runner wake up and handle it in its own
 * context.
 */

typedef enum ScrMsgType { SCR_MSG_ADD,
                          SCR_MSG_SPELL_CHECK,
                          SCR_MSG_FINALIZE } ScrMsgType;

/*
 * The message queue is a linked-list of messages.
 */
typedef struct ScrMsg
{
    ScrMsgType type;
    void *arg;
    struct ScrMsg *next;
} ScrMsg;

/*
 * The spell-check message has a more complicated arg element, so hold it in a
 * struct.
 */
typedef struct ScrSpellCheckArg
{
    char *text;
    SpellCheckerCallback callback;
} ScrSpellCheckArg;

struct _SpellCheckerRunner
{
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    ScrMsg *csrMsgHead;
    ScrMsg *csrMsgTail;

    SpellCheckerDataHandle data;

    int isRunning;
};

static ScrMsg* createScrMsg(ScrMsgType type, void *arg)
{
    ScrMsg *msg = malloc(sizeof(ScrMsg));
    if (msg)
    {
        msg->type = type;
        msg->arg = arg;
        msg->next = NULL;
    }
    else
    {
        printf("Failed allocating memory for ScrMsg\n");
    }

    return msg;
}

static int scrDoSpellCheck(SpellCheckerDataHandle data, void *arg)
{
    ScrSpellCheckArg *msg = (ScrSpellCheckArg*) arg;

    char *text = msg->text;

    char *word = strtok(text, " ,.-/';:()[]{}\n\"");
    while (word)
    {
        if (!scdHasWord(data, word))
            msg->callback(word);

        word = strtok(NULL, " ,.-/';:()[]{}\n\"");
    }

    free(msg->text);

    return 0;
}

static void * thread_runner(void *arg)
{
    SpellCheckerRunnerHandle runner = (SpellCheckerRunnerHandle) arg;
    int handled = 1;
    while (handled && runner->isRunning)
    {
        pthread_mutex_lock(&runner->mutex);

        while (!runner->csrMsgHead)
            pthread_cond_wait(&runner->cond, &runner->mutex);
        switch(runner->csrMsgHead->type)
        {
        case SCR_MSG_ADD:
            scdAddWord(runner->data, (const char*)runner->csrMsgHead->arg);
            free(runner->csrMsgHead->arg);
            break;
        case SCR_MSG_SPELL_CHECK:
            scrDoSpellCheck(runner->data, runner->csrMsgHead->arg);
            free(runner->csrMsgHead->arg);
            break;
        case SCR_MSG_FINALIZE:
            runner->isRunning = 0;
            break;
        default:
            printf("Internal error: unrecognized message type (%d)!\n",
                   runner->csrMsgHead->type);
            handled = 0;
        }

        if (handled)
        {
            ScrMsg *next = runner->csrMsgHead->next;
            free(runner->csrMsgHead);
            runner->csrMsgHead = next;
        }

        pthread_mutex_unlock(&runner->mutex);
    }

    return NULL;
}

static void csrPushMsg(SpellCheckerRunnerHandle runner, ScrMsgType type,
                       void *arg)
{
    pthread_mutex_lock(&runner->mutex);
    if (!runner->csrMsgHead)
    {
        runner->csrMsgTail = runner->csrMsgHead = createScrMsg(type, arg);
    }
    else
    {
        if (!runner->csrMsgTail->next)
        {
            runner->csrMsgTail->next = createScrMsg(type, arg);
            runner->csrMsgTail = runner->csrMsgTail->next;
        }
        else
        {
            printf("Something's wrong...\n");
        }
    }
    pthread_cond_broadcast(&runner->cond);
    pthread_mutex_unlock(&runner->mutex);
}

SpellCheckerRunnerHandle scrInit(void)
{
    SpellCheckerRunnerHandle runner =
        malloc(sizeof(struct _SpellCheckerRunner));
    if (!runner)
    {
        printf("Failed allocating memory for SpellCheckerRunnerHandle\n");
        return NULL;
    }

    runner->data = scdInit();
    if (!runner->data)
    {
        printf("Failed initializing data model\n");
        free (runner);
        return NULL;
    }

    runner->isRunning = 1;

    pthread_mutex_init(&runner->mutex, NULL);
    pthread_cond_init(&runner->cond, NULL);
    pthread_create(&runner->thread, NULL, thread_runner, runner);

    runner->csrMsgHead = NULL;
    runner->csrMsgTail = NULL;

    return runner;
}

int scrFinalize(SpellCheckerRunnerHandle runner)
{
    if (!runner)
    {
        printf("NULL runner pointer\n");
        return -1;
    }

    if (!runner->isRunning)
    {
        printf("Nothing to see here... Carry on.\n");
        return 0;
    }

    csrPushMsg(runner, SCR_MSG_FINALIZE, NULL);

    pthread_join(runner->thread, NULL);

    scdFinalize(runner->data);

    free(runner);

    return 0;
}

int scrAddWord(SpellCheckerRunnerHandle runner, const char *word)
{
    if (!runner || !word)
    {
        printf("Illegal argument(s) passed to scrAddWord\n");
        return -1;
    }

    if (!runner->isRunning)
    {
        printf("Runner is not running. Free and call init again to get a valid"
               "one\n");
        return 0;
    }

    char *arg = malloc(strlen(word)+1);
    if (!arg)
    {
        printf("Failed allocating memory for word\n");
        return -1;
    }

    strcpy(arg, word);

    csrPushMsg(runner, SCR_MSG_ADD, arg);

    return 0;
}

int scrRunSpellCheck(SpellCheckerRunnerHandle runner, const char *text,
                     SpellCheckerCallback callback)
{
    if (!runner || !text || !callback)
    {
        printf("Illegal argument(s) passed to scrRunSpellCheck\n");
        return -1;
    }

    if (!runner->isRunning)
    {
        printf("Runner is not running. Free and call init again to get a valid"
               "one\n");
        return 0;
    }

    char *copiedText = malloc(strlen(text)+1);
    if (!copiedText)
    {
        printf("Failed allocating memory for text\n");
        return -1;
    }

    strcpy(copiedText, text);

    ScrSpellCheckArg *msgArg = malloc(sizeof(ScrSpellCheckArg));
    if (!msgArg)
    {
        printf("Failed allocating memory for spell-check message\n");
        free(copiedText);
        return -1;
    }

    msgArg->text = copiedText;
    msgArg->callback = callback;
    csrPushMsg(runner, SCR_MSG_SPELL_CHECK, msgArg);

    return 0;
}
