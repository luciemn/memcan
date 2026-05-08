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

static void keyword_print(const KeywordNode *head) {
    printf("Keywords loaded:\n");
    for (const KeywordNode *node = head; node != NULL; node = node->next) {
        printf("- %s\n", node->word);
    }
}

static void keyword_free(KeywordNode *head) {
    KeywordNode *node = head;
    while (node) {
        KeywordNode *next = node->next;
        free(node);
        node = next;
    }
}

static KeywordNode *keyword_load(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror(filename);
        exit(EXIT_FAILURE);
    }

    KeywordNode *head = NULL;
    char line[MAX_WORD];
    int count = 0;

    while (fgets(line, sizeof(line), fp)) {
        char *nl = strpbrk(line, "\r\n");
        if (nl) *nl = '\0';

        if (line[0] == '\0') continue;
        if (line[0] == '#') continue;

        head = keyword_push(head, line);
        count++;
    }

    fclose(fp);
    printf("[*] Loaded %d keyword(s) from '%s'.\n", count, filename);
    return head;
}

static MatchNode *match_push(MatchNode *head,
                             const char *keyword,
                             long offset,
                             const unsigned char *buf,
                             size_t bytes_read,
                             const unsigned char *ptr,
                             size_t klen,
                             int ctx_len)
{
    MatchNode *node = (MatchNode *)malloc(sizeof(MatchNode));
    if (!node) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    strncpy(node->keyword, keyword, MAX_WORD - 1);
    node->keyword[MAX_WORD - 1] = '\0';
    node->offset = offset;
    node->context_len = ctx_len;

    node->context_before = (char *)malloc((size_t)ctx_len + 1);
    node->context_after  = (char *)malloc((size_t)ctx_len + 1);
    if (!node->context_before || !node->context_after) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    memset(node->context_before, 0, (size_t)ctx_len + 1);
    memset(node->context_after,  0, (size_t)ctx_len + 1);

    long left = (long)(ptr - buf);
    int before_len = (left >= ctx_len) ? ctx_len : (int)left;
    memcpy(node->context_before, ptr - before_len, (size_t)before_len);

    const unsigned char *after_start = ptr + klen;
    long right = (long)((buf + bytes_read) - after_start);
    int after_len = (right >= ctx_len) ? ctx_len : (int)right;
    memcpy(node->context_after, after_start, (size_t)after_len);

    node->next = head;
    return node;
}

static void match_free(MatchNode *head) {
    MatchNode *node = head;
    while (node) {
        MatchNode *next = node->next;
        free(node->context_before);
        free(node->context_after);
        free(node);
        node = next;
    }
}