/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ast_node {
    struct ast_node* left;
    struct ast_node* right;
    char data[64];
    int id;
} ast_node_t;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    printf("Destructor: Cleaning up ASAN test\n");
}

/* Recursive function with memory operations */
static ast_node_t* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ast_node_t* node = (ast_node_t*)malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ast_node_t));
    
    node->id = id;
    
    /* Fill data with pattern using builtin memcpy */
    char pattern[64];
    __builtin_memset(pattern, 'A' + (id % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(node->data));
    
    /* Recursive creation */
    node->left = create_ast(depth - 1, id * 2);
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    return node;
}

/* Function with goto and memory operations */
static void process_with_goto(ast_node_t* dest, ast_node_t* src) {
    int state = 0;
    
    /* Jump into memory operation block */
    if (src && dest) {
        goto mem_operation;
    }
    
    normal_path:
        __builtin_memset(dest->data, 'X', 32);
        return;
    
    mem_operation:
        /* This tests flow-sensitivity of ASAN logic */
        __builtin_memmove(dest->data, src->data, 32);
        
        /* Jump out */
        if (state++ < 2) {
            goto normal_path;
        }
}

/* Parallel memory dispatch function */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf1[128];
        char local_buf2[128];
        
        /* Initialize with builtins */
        __builtin_memset(local_buf1, thread_id + '0', sizeof(local_buf1));
        
        /* Copy between buffers */
        __builtin_memcpy(local_buf2, local_buf1, sizeof(local_buf1));
        
        /* Move data around */
        __builtin_memmove(local_buf1 + 32, local_buf1, 64);
        
        #pragma omp barrier
        
        /* Verify the copy */
        int mismatch = 0;
        for (size_t i = 0; i < sizeof(local_buf1); i++) {
            if (local_buf1[i] != local_buf2[(i + 32) % sizeof(local_buf2)]) {
                mismatch = 1;
                break;
            }
        }
        
        #pragma omp critical
        {
            printf("Thread %d: Memory ops completed, mismatch=%d\n", 
                   thread_id, mismatch);
        }
    }
}

/* Complex token processing */
static uint64_t process_tokens(const char** tokens, size_t count) {
    uint64_t hash = 0xDEADBEEF;
    char buffer[256];
    
    for (size_t i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]);
        
        /* Use volatile to control length */
        volatile size_t copy_len = len;
        if (copy_len > sizeof(buffer) - 1) {
            copy_len = sizeof(buffer) - 1;
        }
        
        /* Clear buffer with builtin */
        __builtin_memset(buffer, 0, sizeof(buffer));
        
        /* Copy token with builtin */
        __builtin_memcpy(buffer, tokens[i], copy_len);
        
        /* Compute simple hash */
        for (size_t j = 0; j < copy_len; j++) {
            hash = (hash * 31) + buffer[j];
        }
        
        /* Move data within buffer */
        if (i > 0) {
            __builtin_memmove(buffer + 16, buffer, 32);
        }
    }
    
    return hash;
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Initialize complex token array */
    const char* tokens[] = {
        "memcpy", "memset", "memmove", "asan", "hwasan",
        "instrumentation", "redzone", "builtin", "coverage"
    };
    size_t token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Process tokens with memory operations */
    uint64_t token_hash = process_tokens(tokens, token_count);
    printf("Token hash: 0x%016llX\n", (unsigned long long)token_hash);
    
    /* Phase 2: Create and process AST */
    ast_node_t* ast_root = create_ast(3, 1);
    if (ast_root) {
        /* Process with goto jumps */
        process_with_goto(ast_root->left, ast_root->right);
        
        /* Copy between nodes */
        if (ast_root->left && ast_root->right) {
            __builtin_memcpy(ast_root->left->data + 16, 
                           ast_root->right->data, 32);
        }
        
        /* Free AST recursively */
        /* ... (omitted for brevity, would need proper free function) */
    }
    
    /* Phase 3: Execute parallelized memory dispatch */
    printf("\nParallel memory operations:\n");
    parallel_memory_ops();
    
    /* Phase 4: Dynamic memory operations with volatile sizes */
    volatile size_t dyn_size = g_mem_size;
    char* dyn_buf1 = (char*)malloc(dyn_size);
    char* dyn_buf2 = (char*)malloc(dyn_size);
    
    if (dyn_buf1 && dyn_buf2) {
        /* Initialize with builtins */
        __builtin_memset(dyn_buf1, 0xAA, dyn_size);
        __builtin_memset(dyn_buf2, 0xBB, dyn_size);
        
        /* Copy and move */
        __builtin_memcpy(dyn_buf2, dyn_buf1, dyn_size / 2);
        __builtin_memmove(dyn_buf1 + dyn_size / 4, dyn_buf1, dyn_size / 2);
        
        /* Verify */
        int errors = 0;
        for (size_t i = 0; i < dyn_size / 2; i++) {
            if (dyn_buf1[i + dyn_size / 4] != 0xAA) errors++;
        }
        printf("Dynamic memory errors: %d\n", errors);
        
        free(dyn_buf1);
        free(dyn_buf2);
    }
    
    /* Final verification sum */
    uint64_t final_sum = token_hash + g_init_flag;
    printf("\nFinal verification sum: 0x%016llX\n", 
           (unsigned long long)final_sum);
    
    printf("=== Test Complete ===\n");
    return (final_sum != 0) ? 0 : 1;
}
