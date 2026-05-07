#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define CHUNK_SIZE (1024 * 1024)
#define MAX_WORD 256

typedef struct KeywordNode {
    char word[MAX_WORD];
    struct KeywordNode *next;
} KeywordNode;

typedef struct MatchNode {
    char keyword[MAX_WORD];
    long offset;
    char *context_before;
    char *context_after;
    int context_len;
    struct MatchNode *next;
} MatchNode;