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

static FILE *open_image(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror(filename);
        exit(EXIT_FAILURE);
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        perror("fseek");
        exit(EXIT_FAILURE);
    }

    long size = ftell(fp);
    if (size < 0) {
        perror("ftell");
        exit(EXIT_FAILURE);
    }

    rewind(fp);

    printf("[*] Image: '%s' size: %ld bytes", filename, size);
    if (size == 1024 * 1024) printf(" (1 MB)");
    printf("\n");

    return fp;
}

static KeywordNode *keyword_push(KeywordNode *head, const char *word) {
    KeywordNode *node = (KeywordNode *)malloc(sizeof(KeywordNode));
    if (!node) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    strncpy(node->word, word, MAX_WORD - 1);
    node->word[MAX_WORD - 1] = '\0';
    node->next = head;
    return node;
}