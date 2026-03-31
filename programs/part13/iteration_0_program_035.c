/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char token_pool[4096];
static volatile int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_token_pool(void) {
    /* Initialize with pattern using builtin memset */
    __builtin_memset(token_pool, 0xAA, sizeof(token_pool));
    
    /* Force redirection by using all three builtins */
    char temp[128];
    __builtin_memset(temp, 0x55, 128);
    __builtin_memcpy(token_pool + 512, temp, 128);
    __builtin_memmove(token_pool + 256, token_pool + 512, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_tokens(void) {
    /* Use volatile to ensure builtin calls aren't optimized away */
    if (volatile_flag) {
        __builtin_memset(token_pool, 0, sizeof(token_pool));
    }
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with builtin memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Create pattern in data */
    char pattern[32];
    for (int i = 0; i < 32; i++) pattern[i] = (char)(id + i);
    
    /* Copy pattern using builtin memcpy with volatile length */
    int copy_len = volatile_len % 32;
    if (copy_len > 0) {
        __builtin_memcpy(node->data, pattern, copy_len);
    }
    
    node->id = id;
    node->left = create_ast(depth - 1, id * 2);
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    int state = 0;
    
    /* Jump into block with memmove */
    if (src && dst) {
        goto process_block;
    }
    
skip_block:
    return;
    
process_block:
    /* This block contains builtin memmove */
    char buffer[256];
    __builtin_memcpy(buffer, src->data, 128);
    
    /* Conditional goto */
    if (state == 0) {
        state = 1;
        goto use_memmove;
    }
    
    __builtin_memset(buffer + 128, 0, 128);
    goto skip_block;
    
use_memmove:
    /* Force memmove redirection */
    __builtin_memmove(dst->data, buffer, 128);
    
    /* Jump out */
    goto skip_block;
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread works on its own buffer */
        char local_buf[512];
        char src_buf[512];
        
        /* Initialize with builtin memset */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        __builtin_memset(src_buf, 0xFF, sizeof(src_buf));
        
        /* Copy between buffers */
        int copy_size = (volatile_len + thread_id) % 256;
        if (copy_size > 0) {
            __builtin_memcpy(local_buf, src_buf, copy_size);
        }
        
        /* Move data around */
        if (thread_id % 2 == 0) {
            __builtin_memmove(local_buf + 256, local_buf, 128);
        }
        
        /* Store result in token pool (with synchronization) */
        #pragma omp critical
        {
            int offset = token_index % (sizeof(token_pool) - 512);
            __builtin_memcpy(token_pool + offset, local_buf, 256);
            token_index += 256;
        }
    }
}

/* Multi-stage processing function */
static unsigned long process_ast(ASTNode* root) {
    if (!root) return 0;
    
    unsigned long hash = 0;
    char temp[256];
    
    /* Process current node */
    __builtin_memcpy(temp, root->data, sizeof(root->data));
    
    /* Compute simple hash */
    for (int i = 0; i < 256; i++) {
        hash = hash * 31 + (unsigned char)temp[i];
    }
    
    /* Recursively process children */
    hash += process_ast(root->left);
    hash += process_ast(root->right);
    
    return hash;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    /* Clear data before freeing */
    if (volatile_flag) {
        __builtin_memset(node->data, 0, sizeof(node->data));
    }
    
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Create and process AST */
    ASTNode* ast_root = create_ast(4, 1);
    if (!ast_root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Process with goto jumps */
    if (ast_root->left && ast_root->right) {
        process_with_goto(ast_root->left, ast_root->right);
    }
    
    /* Phase 3: OpenMP parallel operations */
    #ifdef _OPENMP
    printf("Running OpenMP parallel section\n");
    #endif
    parallel_memory_ops();
    
    /* Phase 4: Compute and verify result */
    unsigned long ast_hash = process_ast(ast_root);
    printf("AST hash: %lu\n", ast_hash);
    
    /* Phase 5: Additional builtin calls in cleanup */
    char verify_buf[1024];
    __builtin_memset(verify_buf, 0, sizeof(verify_buf));
    __builtin_memcpy(verify_buf, token_pool, 512);
    __builtin_memmove(verify_buf + 512, verify_buf, 256);
    
    /* Compute final verification hash */
    unsigned long final_hash = 0;
    for (int i = 0; i < sizeof(verify_buf); i++) {
        final_hash = final_hash * 31 + (unsigned char)verify_buf[i];
    }
    printf("Final verification hash: %lu\n", final_hash);
    
    /* Cleanup */
    free_ast(ast_root);
    
    printf("Test completed successfully\n");
    return 0;
}
