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