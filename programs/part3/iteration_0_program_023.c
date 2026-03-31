/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 256;
volatile int volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    /* Force early initialization of ASAN runtime */
    char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
    printf("[Constructor] Initialized buffer\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late(void) {
    printf("[Destructor] Cleanup complete\n");
}

/* Recursive function with memory operations */
static ASTNode* create_tree(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, node->id % 256, sizeof(node->data));
    
    /* Create left subtree with goto-controlled flow */
    int use_goto = (depth % 2 == 0);
    
    if (use_goto) {
        goto create_left;
    }
    
    node->left = create_tree(depth - 1, counter);
    goto skip_left;
    
create_left:
    /* Jump into block with memory operation */
    node->left = create_tree(depth - 2, counter);
    
    /* Use __builtin_memmove within goto block */
    if (node->left) {
        char temp[64];
        __builtin_memmove(temp, node->left->data, sizeof(temp));
        __builtin_memmove(node->left->data, node->data, sizeof(node->data));
        __builtin_memmove(node->data, temp, sizeof(temp));
    }
    
skip_left:
    node->right = create_tree(depth - 1, counter);
    
    /* Copy data between nodes using __builtin_memcpy */
    if (node->left && node->right) {
        size_t copy_len = sizeof(node->data);
        if (volatile_flag) {
            copy_len = volatile_len % sizeof(node->data);
        }
        __builtin_memcpy(node->right->data, node->left->data, copy_len);
    }
    
    return node;
}

/* Parallel memory operation dispatcher */
static void parallel_mem_operations(void) {
    const int num_ops = 1000;
    char* buffers[1000];
    
    #pragma omp parallel for
    for (int i = 0; i < num_ops; i++) {
        buffers[i] = (char*)malloc(128);
        if (!buffers[i]) continue;
        
        /* Mix different builtins in parallel regions */
        switch (i % 3) {
            case 0:
                __builtin_memset(buffers[i], i % 256, 128);
                break;
            case 1:
                if (i > 0) {
                    __builtin_memcpy(buffers[i], buffers[i-1], 64);
                }
                break;
            case 2:
                __builtin_memmove(buffers[i] + 32, buffers[i], 64);
                break;
        }
        
        /* Force non-foldable operation */
        size_t len = (volatile_len + i) % 128;
        if (len > 0) {
            __builtin_memset(buffers[i] + 64, 0xFF, len);
        }
    }
    
    /* Cleanup */
    #pragma omp parallel for
    for (int i = 0; i < num_ops; i++) {
        if (buffers[i]) free(buffers[i]);
    }
}

/* Complex token processing with control flow */
static int process_tokens(char** tokens, int count) {
    int result = 0;
    char buffer[1024];
    
    for (int i = 0; i < count; i++) {
        /* Jump label for goto */
        if (i % 7 == 0) {
            goto memmove_block;
        }
        
        /* Normal memcpy path */
        __builtin_memcpy(buffer + result, tokens[i], strlen(tokens[i]));
        result += strlen(tokens[i]);
        continue;
        
memmove_block:
        /* Goto target with memmove */
        __builtin_memmove(buffer + result, tokens[i], strlen(tokens[i]));
        result += strlen(tokens[i]);
        
        /* Nested goto */
        if (i % 3 == 0) {
            goto skip_adjust;
        }
        
        /* Adjust buffer */
        __builtin_memset(buffer + result, '_', 1);
        result++;
        
skip_adjust:
        continue;
    }
    
    return result;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Recursive tree with memory operations */
    int counter = 1;
    ASTNode* root = create_tree(5, &counter);
    
    /* Phase 2: Parallel memory operations */
    parallel_mem_operations();
    
    /* Phase 3: Token processing with goto */
    char* tokens[] = {"Hello", "World", "ASAN", "HWASAN", "Test", "Program"};
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    int processed = process_tokens(tokens, token_count);
    
    /* Phase 4: Mixed builtin usage with volatile control */
    char src[512], dst[512];
    for (int i = 0; i < 512; i++) {
        src[i] = i % 256;
    }
    
    /* Force all three builtins with volatile lengths */
    size_t len1 = volatile_len % 256;
    size_t len2 = (volatile_len * 2) % 256;
    size_t len3 = (volatile_len / 2) % 256;
    
    __builtin_memset(dst, 0, sizeof(dst));
    __builtin_memcpy(dst, src, len1);
    __builtin_memmove(dst + 128, dst, len2);
    __builtin_memset(dst + 256, 0xFF, len3);
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < 512; i++) {
        hash = (hash * 31 + dst[i]) % 1000000007;
    }
    
    printf("Processed %d token bytes\n", processed);
    printf("Final hash: %lu\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Note: Tree cleanup omitted for brevity - would need recursive free */
    
    return 0;
}
