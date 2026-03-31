/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char g_token_pool[4096];
static volatile size_t g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize token pool with pattern */
    for (size_t i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)(i % 256);
    }
    g_init_flag = 1;
    printf("Constructor: Initialized token pool\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    /* Clear sensitive data */
    __builtin_memset(g_token_pool, 0, sizeof(g_token_pool));
    printf("Destructor: Cleaned up token pool\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(size_t depth, const char* base_data) {
    if (depth == 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy for initialization */
    __builtin_memcpy(node->data, base_data, 64);
    
    /* Use volatile to control size */
    node->size = g_mem_size / (depth + 1);
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        node->left = create_ast(depth - 1, base_data);
        
        /* Jump label for goto */
        create_right:
        node->right = create_ast(depth - 2, base_data);
        
        /* Copy between nodes using __builtin_memmove */
        if (node->left && node->right) {
            volatile size_t copy_size = 32;
            __builtin_memmove(node->right->data, 
                            node->left->data, 
                            copy_size);
        }
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void process_with_goto(ASTNode* node) {
    if (!node) return;
    
    volatile int use_goto = 1;
    
    if (use_goto) {
        goto mem_operation;
    }
    
    /* This block will be jumped into */
    mem_operation:
    {
        char temp[64];
        /* __builtin_memset with volatile size */
        volatile size_t fill_size = 48;
        __builtin_memset(temp, 0xAA, fill_size);
        
        /* __builtin_memcpy with conditional goto */
        if (node->left) {
            __builtin_memcpy(node->data, temp, 32);
            goto after_copy;
        }
        
        after_copy:
        /* Additional operation */
        __builtin_memmove(temp, node->data, 16);
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    const size_t block_size = 128;
    char* buffers[4];
    
    /* Allocate buffers */
    for (int i = 0; i < 4; i++) {
        buffers[i] = (char*)malloc(block_size);
        if (!buffers[i]) return;
    }
    
    #pragma omp parallel num_threads(2)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread uses different builtins */
        if (tid == 0) {
            /* Thread 0: memset and memcpy */
            __builtin_memset(buffers[0], tid, block_size);
            __builtin_memcpy(buffers[1], buffers[0], block_size / 2);
        } else {
            /* Thread 1: memmove and memset */
            __builtin_memset(buffers[2], tid, block_size);
            __builtin_memmove(buffers[3], buffers[2], block_size / 2);
        }
        
        /* Barrier to ensure both threads complete */
        #pragma omp barrier
        
        /* Cross-thread memory operation */
        if (tid == 0) {
            volatile size_t cross_size = 32;
            __builtin_memcpy(buffers[2] + 64, buffers[1], cross_size);
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        free(buffers[i]);
    }
}

/* Complex token processing with builtins */
static size_t process_tokens(void) {
    size_t hash = 0;
    char buffer[256];
    volatile size_t process_size = 128;
    
    /* Process tokens in chunks */
    for (size_t i = 0; i < sizeof(g_token_pool); i += process_size) {
        size_t chunk_size = (i + process_size <= sizeof(g_token_pool)) ? 
                           process_size : sizeof(g_token_pool) - i;
        
        /* Use all three builtins in sequence */
        __builtin_memset(buffer, 0, sizeof(buffer));
        __builtin_memcpy(buffer, g_token_pool + i, chunk_size);
        
        /* Conditional memmove */
        if (i > 0) {
            __builtin_memmove(buffer + 64, buffer, 32);
        }
        
        /* Compute simple hash */
        for (size_t j = 0; j < chunk_size; j++) {
            hash = (hash * 31 + buffer[j]) % 1000000007;
        }
    }
    
    return hash;
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Wait for constructor */
    while (!g_init_flag) {}
    
    /* Create recursive structure */
    ASTNode* root = create_ast(4, "BaseDataForAST");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process with goto jumps */
    process_with_goto(root);
    
    /* Execute parallel operations */
    parallel_memory_ops();
    
    /* Process tokens */
    size_t final_hash = process_tokens();
    printf("Token processing hash: %zu\n", final_hash);
    
    /* Additional builtin calls in different contexts */
    {
        char final_buffer[512];
        volatile size_t final_size = 256;
        
        /* Chain of builtins */
        __builtin_memset(final_buffer, 0, sizeof(final_buffer));
        __builtin_memcpy(final_buffer, root->data, 64);
        __builtin_memmove(final_buffer + 128, final_buffer, 64);
        
        /* Use in loop with volatile control */
        for (volatile int i = 0; i < 3; i++) {
            __builtin_memset(final_buffer + i * 64, i, 32);
        }
    }
    
    /* Cleanup */
    /* Recursive free would be needed for full implementation */
    free(root);
    
    printf("Test completed successfully\n");
    return 0;
}
