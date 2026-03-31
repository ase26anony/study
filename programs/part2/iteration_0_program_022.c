/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 256;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 192;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    char *data;
    size_t data_len;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_constructor(void) {
    printf("Constructor: Initializing ASAN/HWASAN environment\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    printf("Destructor: Program completed\n");
}

/* Recursive AST creation */
ASTNode* create_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = depth % 3;
    node->value = depth * 100;
    node->data_len = (size_t)(depth + 1) * 32;
    node->data = (char*)malloc(node->data_len);
    
    /* Use __builtin_memset to initialize data */
    if (node->data) {
        __builtin_memset(node->data, 'A' + depth, node->data_len);
    }
    
    node->left = create_ast(depth + 1, max_depth);
    node->right = create_ast(depth + 2, max_depth);
    
    return node;
}

/* Recursive AST copy using __builtin_memcpy */
void copy_ast_data(ASTNode *dest, const ASTNode *src) {
    if (!dest || !src) return;
    
    /* Force memcpy redirection with volatile length */
    volatile size_t copy_len = src->data_len;
    if (dest->data && src->data && copy_len > 0) {
        __builtin_memcpy(dest->data, src->data, copy_len);
    }
    
    copy_ast_data(dest->left, src->left);
    copy_ast_data(dest->right, src->right);
}

/* Function with goto jumps around __builtin_memmove */
void memmove_with_goto(char *dest, char *src, size_t len) {
    int use_memmove = 1;
    
    if (len == 0) goto skip_memmove;
    
    /* Jump into memmove block */
    goto do_memmove;
    
memmove_block:
    /* This label is after the goto target */
    printf("Memmove block executed\n");
    return;
    
do_memmove:
    /* Force memmove redirection */
    __builtin_memmove(dest, src, len);
    
    /* Jump out of block */
    goto memmove_block;
    
skip_memmove:
    printf("Skipped memmove\n");
}

/* OpenMP parallel memory operations */
void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        size_t buffer_size = 1024;
        char *src = (char*)malloc(buffer_size);
        char *dest = (char*)malloc(buffer_size);
        
        if (src && dest) {
            /* Each thread uses different builtins */
            __builtin_memset(src, thread_id + '0', buffer_size);
            
            /* Use volatile length */
            volatile size_t copy_len = g_memcpy_len;
            __builtin_memcpy(dest, src, copy_len);
            
            /* In-place shift with memmove */
            __builtin_memmove(src + 100, src, buffer_size - 100);
        }
        
        free(src);
        free(dest);
    }
}

/* Complex token processing with memory builtins */
void process_tokens(char **tokens, int token_count) {
    char buffer[4096];
    char *current = buffer;
    
    for (int i = 0; i < token_count; i++) {
        size_t token_len = strlen(tokens[i]);
        
        /* Mix of memory operations */
        if (i % 3 == 0) {
            __builtin_memcpy(current, tokens[i], token_len);
            current += token_len;
        } else if (i % 3 == 1) {
            __builtin_memset(current, '*', token_len);
            current += token_len;
        } else {
            /* Create overlapping region for memmove */
            if (current > buffer + 100) {
                __builtin_memmove(current - 50, current - 100, 50);
            }
            __builtin_memcpy(current, tokens[i], token_len);
            current += token_len;
        }
    }
    
    /* Null terminate */
    __builtin_memset(current, '\0', 1);
}

int main(void) {
    printf("=== ASAN/HWASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Basic built-in calls with volatile lengths */
    char src[512], dest[512];
    
    __builtin_memset(src, 'X', sizeof(src));
    __builtin_memcpy(dest, src, g_memcpy_len);
    __builtin_memmove(src + 100, src, g_memmove_len);
    
    /* Phase 2: AST operations */
    ASTNode *ast1 = create_ast(0, 4);
    ASTNode *ast2 = create_ast(0, 4);
    
    if (ast1 && ast2) {
        copy_ast_data(ast2, ast1);
    }
    
    /* Phase 3: Goto-controlled memmove */
    char buffer1[256], buffer2[256];
    __builtin_memset(buffer1, 'G', sizeof(buffer1));
    memmove_with_goto(buffer2, buffer1, sizeof(buffer1));
    
    /* Phase 4: OpenMP parallel section */
    parallel_memory_ops();
    
    /* Phase 5: Token processing */
    char *tokens[] = {"Hello", "World", "ASAN", "HWASAN", "Test", "Program"};
    process_tokens(tokens, sizeof(tokens)/sizeof(tokens[0]));
    
    /* Verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(dest); i++) {
        hash = (hash * 31) + dest[i];
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("All memory operations completed\n");
    
    /* Cleanup */
    /* Note: Real program would free AST nodes recursively */
    
    return 0;
}
