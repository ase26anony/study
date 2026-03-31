/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_globals(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->id = (*counter)++;
    
    /* Use volatile to control string length */
    volatile int str_len = 32;
    char temp[64];
    
    /* Create pattern with builtin memcpy */
    for (int i = 0; i < str_len; i++) {
        temp[i] = 'A' + (i % 26);
    }
    temp[str_len] = '\0';
    
    /* Copy with goto for flow control */
    int copy_done = 0;
    copy_start:
    if (!copy_done) {
        __builtin_memcpy(node->data, temp, str_len + 1);
        copy_done = 1;
        goto copy_end;
    }
    copy_end:
    
    /* Recursive creation with conditional memmove */
    node->left = create_ast(depth - 1, counter);
    node->right = create_ast(depth - 1, counter);
    
    /* Move data between nodes if right exists */
    if (node->right) {
        volatile size_t move_size = sizeof(node->data);
        if (g_use_memmove) {
            __builtin_memmove(node->right->data, node->data, move_size);
        }
    }
    
    return node;
}

/* Function with goto jumping into memory block */
static void test_goto_memcpy(void* dest, const void* src, size_t n) {
    int state = 0;
    
    goto_label:
    if (state == 0) {
        state = 1;
        /* Jump into memory operation block */
        goto mem_block;
    } else if (state == 1) {
        /* Normal path */
        __builtin_memcpy(dest, src, n);
        return;
    }
    
    mem_block:
    {
        /* This block should be reached via goto */
        volatile size_t local_n = n;
        __builtin_memmove(dest, src, local_n);
        goto exit_block;
    }
    
    exit_block:
    return;
}

/* Parallel memory operations */
static unsigned long process_ast_parallel(ASTNode* root) {
    unsigned long hash = 0;
    
    #pragma omp parallel reduction(+:hash)
    {
        #pragma omp single
        {
            printf("OpenMP: %d threads\n", omp_get_num_threads());
        }
        
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            char buffer1[128];
            char buffer2[128];
            
            /* Initialize with builtin memset */
            volatile size_t init_size = g_mem_size % 128;
            __builtin_memset(buffer1, i, init_size);
            __builtin_memset(buffer2, 255 - i, init_size);
            
            /* Copy with different builtins */
            if (i % 3 == 0) {
                __builtin_memcpy(buffer2, buffer1, init_size);
            } else if (i % 3 == 1) {
                __builtin_memmove(buffer2, buffer1, init_size);
            }
            
            /* Compute simple hash */
            for (size_t j = 0; j < init_size; j++) {
                hash += buffer1[j] + buffer2[j];
            }
        }
    }
    
    return hash;
}

/* Multi-stage initialization */
static void initialize_token_array(char tokens[][32], int count) {
    volatile int pattern_len = 16;
    
    for (int i = 0; i < count; i++) {
        char pattern[32];
        
        /* Create pattern */
        for (int j = 0; j < pattern_len; j++) {
            pattern[j] = '0' + ((i + j) % 10);
        }
        pattern[pattern_len] = '\0';
        
        /* Copy with builtin */
        __builtin_memcpy(tokens[i], pattern, pattern_len + 1);
        
        /* Occasionally use memmove */
        if (i > 0 && (i % 7 == 0)) {
            __builtin_memmove(tokens[i], tokens[i-1], pattern_len);
        }
    }
}

int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Stage 1: Initialize token array */
    char tokens[20][32];
    initialize_token_array(tokens, 20);
    
    /* Stage 2: Create recursive AST */
    int counter = 0;
    ASTNode* root = create_ast(4, &counter);
    printf("Created AST with %d nodes\n", counter);
    
    /* Stage 3: Test goto with memory operations */
    char src[64], dest[64];
    volatile size_t test_size = 48;
    
    __builtin_memset(src, 'X', test_size);
    test_goto_memcpy(dest, src, test_size);
    
    /* Verify the copy */
    int verify = 1;
    for (size_t i = 0; i < test_size; i++) {
        if (dest[i] != 'X') {
            verify = 0;
            break;
        }
    }
    printf("Goto memcpy test: %s\n", verify ? "PASS" : "FAIL");
    
    /* Stage 4: Parallel processing */
    unsigned long hash = process_ast_parallel(root);
    printf("Parallel hash result: %lu\n", hash);
    
    /* Stage 5: Additional builtin stress tests */
    {
        char buffer1[256];
        char buffer2[256];
        volatile size_t opsize = g_mem_size;
        
        /* Chain of memory operations */
        __builtin_memset(buffer1, 0xAA, opsize);
        __builtin_memcpy(buffer2, buffer1, opsize);
        __builtin_memset(buffer1, 0xBB, opsize);
        __builtin_memmove(buffer2, buffer1, opsize / 2);
        
        /* Final verification sum */
        unsigned long sum = 0;
        for (size_t i = 0; i < opsize; i++) {
            sum += buffer1[i] + buffer2[i];
        }
        printf("Final buffer sum: %lu\n", sum);
    }
    
    /* Cleanup */
    /* Note: In real code, we would free the AST recursively */
    
    printf("=== Test Complete ===\n");
    return 0;
}
