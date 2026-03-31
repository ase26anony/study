/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

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
static void init_asan_environment(void) {
    /* Initialize token pool with pattern */
    for (size_t i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)((i % 26) + 'A');
    }
    
    /* Force early initialization of memory functions */
    char buffer[32];
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final memory operations */
    char final_buf[64];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
    __builtin_memmove(final_buf + 32, final_buf, 32);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* src, size_t depth) {
    if (depth > 3) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile size */
    size_t copy_size = g_mem_size % 64;
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    if (src) {
        /* Conditional goto for flow control */
        if (depth % 2 == 0) {
            goto copy_block;
        }
        
        __builtin_memcpy(node->data, src, copy_size);
        goto skip_copy;
        
    copy_block:
        __builtin_memmove(node->data, src, copy_size);
        
    skip_copy:
        node->size = copy_size;
    }
    
    /* Recursive creation with goto jumps */
    if (depth < 3) {
        node->left = create_ast_node(node->data, depth + 1);
        
        /* Jump around memory operation */
        if (node->left) {
            goto setup_right;
        }
        
        __builtin_memset(node->data + 32, 0xCC, 16);
        return node;
        
    setup_right:
        node->right = create_ast_node(node->data + 16, depth + 1);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Process AST with memory operations */
static size_t process_ast(ASTNode* root, int use_parallel) {
    if (!root) return 0;
    
    size_t hash = 0;
    
    /* OpenMP parallel region */
    #pragma omp parallel if(use_parallel) reduction(+:hash)
    {
        /* Each thread processes memory */
        char local_buf[128];
        volatile size_t local_size = g_mem_size % 128;
        
        /* Force builtin calls in parallel context */
        __builtin_memset(local_buf, 0x5A, sizeof(local_buf));
        
        #pragma omp for
        for (int i = 0; i < 4; i++) {
            char thread_buf[64];
            
            /* Varied memory operations */
            switch (i % 3) {
                case 0:
                    __builtin_memcpy(thread_buf, root->data, 
                                    local_size % sizeof(thread_buf));
                    break;
                case 1:
                    __builtin_memset(thread_buf + 16, i, 32);
                    break;
                case 2:
                    __builtin_memmove(thread_buf, thread_buf + 8, 24);
                    break;
            }
            
            /* Accumulate hash */
            for (size_t j = 0; j < sizeof(thread_buf); j++) {
                hash += (size_t)thread_buf[j];
            }
        }
        
        /* Additional memory operation after loop */
        __builtin_memcpy(local_buf + 64, root->data, 32);
    }
    
    /* Process children recursively */
    hash += process_ast(root->left, use_parallel);
    hash += process_ast(root->right, use_parallel);
    
    return hash;
}

/* Memory dispatch with complex flow */
static void memory_dispatch_operation(void) {
    char buffers[3][256];
    volatile int selector = 0;
    
    /* Initialize buffers with different patterns */
    for (int i = 0; i < 3; i++) {
        __builtin_memset(buffers[i], i * 0x33, sizeof(buffers[i]));
    }
    
    /* Complex goto-based flow with memory operations */
    selector = (int)(g_mem_size % 3);
    
    if (selector == 0) {
        goto memcpy_case;
    } else if (selector == 1) {
        goto memset_case;
    } else {
        goto memmove_case;
    }
    
memcpy_case:
    __builtin_memcpy(buffers[0] + 64, buffers[1], 128);
    goto cleanup;
    
memset_case:
    __builtin_memset(buffers[1] + 128, 0xAA, 64);
    goto cleanup;
    
memmove_case:
    __builtin_memmove(buffers[2], buffers[2] + 32, 192);
    /* Fall through */
    
cleanup:
    /* Cross-buffer operations */
    __builtin_memcpy(buffers[0], buffers[2], 64);
    __builtin_memset(buffers[1] + 192, 0, 64);
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Initialize and create AST */
    ASTNode* root = create_ast_node(g_token_pool, 0);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Process AST in parallel */
    size_t hash1 = process_ast(root, 1);  /* With OpenMP */
    
    /* Phase 3: Memory dispatch with goto flow */
    memory_dispatch_operation();
    
    /* Phase 4: Process AST sequentially */
    size_t hash2 = process_ast(root, 0);  /* Without OpenMP */
    
    /* Phase 5: Complex nested memory operations */
    char nested_buf[512];
    volatile size_t offset = g_mem_size % 256;
    
    __builtin_memset(nested_buf, 0, sizeof(nested_buf));
    __builtin_memcpy(nested_buf + offset, root->data, 64);
    __builtin_memmove(nested_buf, nested_buf + 128, 256);
    __builtin_memset(nested_buf + 384, 0xEE, 128);
    
    /* Calculate final result */
    size_t final_hash = hash1 + hash2;
    
    for (size_t i = 0; i < sizeof(nested_buf); i += 16) {
        final_hash += (size_t)nested_buf[i];
    }
    
    printf("Test completed. Final hash: %zu\n", final_hash);
    printf("Expected range: 10000-50000 (implementation dependent)\n");
    
    /* Cleanup */
    free(root);
    
    return 0;
}
